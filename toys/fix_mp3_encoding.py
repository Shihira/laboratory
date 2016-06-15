#!/usr/bin/env python2

import os
import sys
import eyed3.id3

class Convertor(object):
    FIELDS = [ "artist", "title", "album", "album_artist" ]
    CHARSET_PAIRS = [
            ("latin1", "utf8"),
            ("latin1", "gbk"),
            ("latin1", "iso-2022-jp"),
            ("latin1", "big5"),
        ]

    @staticmethod
    def _is_text_unreadable(text):
        if not isinstance(text, unicode):
            raise TypeError("text is required to be a unicode.")
        latin_letter = 0
        euro_letter = 0
        for c in text:
            if c > u'\u007f' and c < u'\u0100':
                euro_letter += 1
            if c < u'\u007f':
                latin_letter += 1

        #return euro_letter > latin_letter
        return bool(euro_letter)


    def __init__(self, fn):
        self.tag = eyed3.id3.tag.Tag()
        self.filename = fn
        self.tag.parse(fn)

    def try_charset_pairs(self, text):
        def cand_cmp(x, y):
            return len(y) - len(x)

        candidate = []
        for i, cs in enumerate(self.CHARSET_PAIRS):
            from_cs, to_cs = cs
            conv_text = text.encode(from_cs, "ignore").decode(to_cs, "ignore")
            candidate += [ conv_text ]

        candidate.sort(cmp=cand_cmp)

        for conv_text in candidate:
            print((u"%-30s -> %-30s" % (repr(text), conv_text))
                    .encode("utf8", "ignore"))
            yn = raw_input("is it right[y/n]: ")

            if yn not in ("y", "yes", ""): continue
            else: return conv_text

        raise RuntimeError("Not converted: %s", repr(text))

    def start_conversion(self):
        for field in self.FIELDS:
            field_text = getattr(self.tag, field, None)

            if field_text is None or not self._is_text_unreadable(field_text):
                continue

            conv_field_text = self.try_charset_pairs(field_text)

            if conv_field_text is not None:
                setattr(self.tag, field, conv_field_text)

        self.tag.save(version=eyed3.id3.ID3_V2_4, encoding="utf8")

if __name__ == '__main__':
    if len(sys.argv) <= 1:
        print("USAGE: %s <mp3-file>" % os.path.basename(sys.argv[0]))
        exit(-1)

    print("Converting %s ..." % sys.argv[1])

    try:
        convertor = Convertor(sys.argv[1])
        convertor.start_conversion()
    except Exception as e:
        print("%s: Error[%s]" % (sys.argv[0], repr(e)))
        file("conv.log", "a").write("%s: Error[%s]" % (sys.argv[0], repr(e)))


