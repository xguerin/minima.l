(load
	"@lib/append.l"
	"@lib/iter.l"
	"@lib/join.l"
	"@lib/split.l"
	"@lib/trim.l"
	'(io in out prinl printl readline)
	'(iterators foldl)
	'(manips cadr caddr cddr)
	'(logic = or)
	'(math >)
	'(std \ <- car cons if len let list match nil? prog unless when while)
	'(unix accept close connect listen select))

(use
	'(io in out prinl printl readline)
	'(iterators foldl)
	'(manips cadr caddr cddr)
	'(logic = or)
	'(math >)
	'(std \ <- car cons if len let list match nil? prog unless when while)
	'(unix accept close connect listen select))

#
# HTTP methods.
#

(def http/headers (result)
	"Parse all HTTP headers."
	(let ((line . (readline))
				(tlne . (trim line)))
		(if (or (nil? tlne) (= line 'EOF))
			(foldl (\ (acc e) 
							 (let (((hd . tl) . (split e ^:))
										 (rem . (join tl ^:)))
								 (cons (cons (trim hd) (trim rem)) acc)))
				NIL result)
			(http/headers (cons tlne result))
			)))

(def http/request ()
	"Process a HTTP request."
	(let ((line . (readline))
				(tlne . (trim line))
				(rqst . (split tlne ^ )))
		(unless (= line 'EOF)
			(match rqst
				#
				# Match HTTP requests.
				#
				((_ _ "HTTP/1.1")	. (let (((op path _) . rqst)
																	(headers . (http/headers NIL)))
															(list op path headers NIL)))
				#
				# Handle invalid request, drop the connection.
				#
				(_	. (prog
								(prinl "Invalid HTTP request")
								NIL))
				))))

(def http/response ()
	"Process a HTTP response."
	(let ((line . (readline))
				(tlne . (trim line))
				(resp . (split tlne ^ )))
		(unless (= line 'EOF)
			(match resp
				#
				# Response is HTTP/1.1
				#
				(("HTTP/1.1" _ . _)	. (let ((code . (cadr resp))
																		(mesg . (join (cddr resp) ^ ))
																		(hdrs . (http/headers NIL)))
																(list code mesg hdrs)))
				#
				# Handle invalid response.
				#
				(_	. (prog
								(prinl "Invalid HTTP response")
								NIL))
				))))

#
# Client methods.
#

(def http/get (address service path fn)
	"Perform a HTTP/1.1 GET to server at ADDRESS on port SERVICE at PATH."
	(let ((fd . (connect address service)))
		(if fd
			(prog
				(out fd
					(prinl "GET " path " HTTP/1.1")
					(prinl "Host: " address)
					(prinl "Connection: close")
					(prinl))
				(in fd
					(let (((code mesg hdrs) . (http/response)))
						(when code
							(fn code mesg hdrs))))
				(close fd))
			(prinl "Connecting to " address " on port " service " failed")
			)))

#
# Server methods.
#

(def http/ingress (srv fn fd)
	"On-read server callback."
	(if (= srv fd)
		(let (((fd . (host . port)) . (accept fd)))
			(prinl "Accepting connection from " host ":" port)
			fd)
		(in fd
			(let (((op path hdrs body) . (http/request)))
				(when op
					(match (fn op path hdrs body)
						(NOT_FOUND . (out fd
													 (prinl "HTTP/1.1 404 Not Found")
													 (prinl "Connection: close")
													 (prinl)))
						(_ . (out fd
									 (prinl "HTTP/1.1 500 Internal Server Error")
									 (prinl "Connection: close")
									 (prinl)))
						))
				NIL
				))))

(def http/serve (port cb)
	"Start a HTTP server on PORT, using CB as a request handler."
	(let ((srv . (listen 8080))
				(fds . (list srv))
				(run . T))
		(while run
			(let (((r e) . (select fds (http/ingress srv cb) NIL)))
				(<- 'fds r)
				(<- 'run (> (len fds) 1))
				))))
