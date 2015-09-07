queens = 8

validateChessboard :: [Int] -> Bool
validateChessboard [] = True
validateChessboard (first:others) =
        -- r: row - rowFirst, c: column
        all (\(r,c) -> c /= first && (abs (c-first)) /= r)
        (zip [1..(length others)] others)

nextRow :: [Int] -> [[Int]]
nextRow rows =
        if validateChessboard rows
        then if length rows < queens
                then foldl (++) [] [(nextRow (n:rows)) | n <- [0..(queens-1)]]
                else [rows]
        else []

result = nextRow []
main = do 
        print result
        print (length result)
