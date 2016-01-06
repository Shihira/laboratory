type JPair = (String, JValue)
type JReason = String
data JValue = JObject [JPair]
            | JArray [JValue]
            | JString String
            | JNumber Double
            | JBoolean Bool
            | JNull

parseJValue :: String -> (JValue, String)
parseJValue s:tail | isSpace(s) = parseJSON tail
parseJValue '{':tail = parseJObject

parseJObject :: String -> [JPair] -> ([JPair], String)
parseJObject s:tail | isSpace(s) = parseJObject tail
parseJObject '"':tail = case parseJObjectKey tail of
    '"':tail = parseJChar ',' $ parseJObjectValue $ parseJChar ':' tail

parseJObjectKey = parseJString
parseJObjectValue = parseValue

parseJChar :: Char -> String -> String
parseJChar _ s:tail | isSpace(s) = parseJObjectColon tail
parseJChar cExpect c:tail | c == cExpect = tail

instance Read JValue where
    read s = 

