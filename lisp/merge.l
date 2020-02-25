(load "iterators.l" '(std \))

(def merge (ded ins lstA lstB)
  "Merge list A into list B using dedup function DED and insert function INS."
  (ded (foldl (\ (acc e) (ins e acc)) lstB lstA)))
