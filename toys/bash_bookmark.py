#!/usr/bin/env python3

import os
import sys
import json

BOOKMARK_PATH = '/home/shihira/.bash_bookmark'
HELP_STR = '''bash_bookmark.py <command>

Available Commands:
    save/b <name>:  save as a Bookmark named <name> current directory
    cd/c <name>:    Change to bookmark named <name>
    del/d <name>:   Delete bookmark named <name>
    list/l:         List saved bookmarks
    push/u:         pUsh current directory into the stack
    pop/o:          pOp from the stack and change to it
    apply/a <n>:    Apply <n>th directory, like pop but no side-effect
    stack/s:        list Stacked directories
    help/h:         Help
'''

class BookmarkManager(object):
    def __init__(self, obj):
        self.__dict__ = obj

        if not hasattr(self, "bookmark"):
            self.bookmark = { }
        if not hasattr(self, "stack"):
            self.stack = { }

    def get_stack(self):
        ppid = str(os.getppid())
        if ppid not in self.stack:
            self.stack[ppid] = []
        return self.stack[ppid]

    def check_process_running(self):
        dead_processes = []
        for pid in self.stack.keys():
            try:
                os.kill(int(pid), 0)
            except:
                dead_processes += [pid]

        for pid in dead_processes:
            del self.stack[pid]

    @classmethod
    def load_bookmark(cls):
        global BOOKMARK_PATH

        try:
            bm_file = open(BOOKMARK_PATH)
            bm = json.load(bm_file)
            bm_file.close()
        except:
            return cls({})

        return cls(bm)

    def save_bookmark(self):
        bm_file = open(BOOKMARK_PATH, "w")
        json.dump(vars(self), bm_file, indent=4)
        bm_file.close()

def get_name():
    if len(sys.argv) <= 2:
        print("Please provide a name: ", file=sys.stderr, end="")
        name = input()
        if not name:
            print("Bad name.", file=sys.stderr)
            exit(-1)
        return name
    else:
        return sys.argv[2]

def get_pwd():
    return os.path.abspath(os.curdir)

def main(argv):
    global HELP_STR

    bm = BookmarkManager.load_bookmark()
    bm.check_process_running()

    if len(argv) <= 1:
        if bm.bookmark:
            argv += ['list']
        if bm.get_stack():
            argv += ['stack']
        else:
            argv += ['h']

    if argv[1] in ['l', 'list']:
        for name, path in bm.bookmark.items():
            print("%8s %s" % (name, path), file=sys.stderr)
    elif argv[1] in ['b', 'save']:
        name = get_name()
        bm.bookmark[name] = get_pwd()
        bm.save_bookmark()
    elif argv[1] in ['c', 'cd']:
        name = get_name()
        path = bm.bookmark[name]
        print(path)
    elif argv[1] in ['d', 'del']:
        name = get_name()
        del bm.bookmark[name]
        bm.save_bookmark()
    elif argv[1] in ['u', 'push']:
        bm.get_stack().append(get_pwd())
        bm.save_bookmark()
    elif argv[1] in ['o', 'pop']:
        path = bm.get_stack().pop()
        print(path)
        bm.save_bookmark()
    elif argv[1] in ['a', 'apply']:
        path = bm.get_stack()[int(get_name()) - 1]
        print(path)
    elif argv[1] in ['s', 'stack']:
        for i, p in enumerate(bm.get_stack()):
            print("%3d %s" % (i + 1, p), file=sys.stderr)
    elif argv[1] in ['h', 'help']:
        print(HELP_STR, file=sys.stderr)

if __name__ == "__main__":
    import sys
    main(sys.argv)

