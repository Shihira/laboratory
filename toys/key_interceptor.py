#!/usr/bin/env python3

import evdev
import time

from evdev import ecodes

class keystroke:
    def __init__(self):
        self.state = 0
        self.keycode = None
        self.keystate = None

    def input(self, ev):
        if self.state == 0 and isinstance(ev, evdev.InputEvent):
            if ev.code != ecodes.MSC_SCAN and ev.type != ecodes.EV_MSC:
                print("Special keypress: %s %s" % (ecodes.EV[ev.type], ecodes.MSC[ev.code]))
            self.state += 1
            return

        if self.state == 1 and isinstance(ev, evdev.KeyEvent):
            self.keycode = ecodes.ecodes[ev.keycode]
            self.keystate = ev.keystate
            self.state += 1
            return

        if self.state == 2 and isinstance(ev, evdev.SynEvent):
            self.state += 1
            return

        self.reset()

    def finished(self):
        return self.state == 3

    def reset(self):
        self.state = 0

def __inject(keycode, keystate):
    global ui

    t = time.time()
    sec = int(t)
    usec = int((t - int(t)) * 1000000)

    ui.write(ecodes.EV_MSC, ecodes.MSC_SCAN, keycode)
    ui.write(ecodes.EV_KEY, keycode, keystate)
    ui.syn()

def __log_keystroke(s, ks=None):
    global automata_state
    ks_str = ("%s %s" % (ecodes.KEY[ks.keycode], ks.keystate)) if ks else ""
    print(automata_state + ": " + s + ks_str)

def mapped(ks):
    __log_keystroke("Inject Mapped ", ks)

    MAP = {
        ecodes.KEY_J: ecodes.KEY_DOWN,
        ecodes.KEY_K: ecodes.KEY_UP,
        ecodes.KEY_H: ecodes.KEY_LEFT,
        ecodes.KEY_L: ecodes.KEY_RIGHT,
        ecodes.KEY_0: ecodes.KEY_HOME,
        ecodes.KEY_4: ecodes.KEY_END,
    }

    __inject(MAP[ks.keycode], ks.keystate)

def inject(ks):
    __log_keystroke("Inject Normal ", ks)
    __inject(ks.keycode, ks.keystate)

def inject_alt_up(ks):
    __log_keystroke("Inject Alt Up")
    __inject(ecodes.KEY_LEFTALT, 0)

def inject_alt_down(ks):
    __log_keystroke("Inject Alt Down")
    __inject(ecodes.KEY_LEFTALT, 1)

automata_state = "Normal"
key_to_map = [
        ecodes.KEY_J,
        ecodes.KEY_K,
        ecodes.KEY_H,
        ecodes.KEY_L,
        ecodes.KEY_0,
        ecodes.KEY_4,
    ]
transit_table = {
    "Normal" : [
        [ (ecodes.KEY_LEFTALT, 1), "Alt" ],
        [ "*", "Normal", inject ],
    ],
    "Alt" : [
        [ (key_to_map, "*"), "Mapped", mapped ],
        [ (ecodes.KEY_LEFTALT, 0), "Normal", inject_alt_down, inject_alt_up],
        [ "*", "Inject", inject_alt_down, inject ],
    ],
    "Inject" : [
        [ (key_to_map, "*"), "Mapped", inject_alt_up, mapped ],
        [ (ecodes.KEY_LEFTALT, 0), "Normal", inject_alt_up ],
        [ "*", "Inject", inject ],
    ],
    "Mapped" : [
        [ (key_to_map, "*"), "Mapped", mapped ],
        [ (ecodes.KEY_LEFTALT, 0), "Normal" ],
        [ "*", "Inject", inject_alt_down, inject ],
    ]
}

def match(inp, fmt):
    if fmt == "*":
        return True
    if isinstance(inp, tuple) and isinstance(fmt, tuple):
        if len(inp) != len(fmt):
            return False
        return all(map(lambda x: match(x[0], x[1]), zip(inp, fmt)))
    if not isinstance(inp, list) and isinstance(fmt, list):
        return inp in fmt

    return inp == fmt

dev = evdev.InputDevice('/dev/input/by-path/platform-i8042-serio-0-event-kbd')
ui = evdev.UInput()

def process(ks):
    global automata_state

    if not ks.finished():
        return

    operations = []
    for opt in transit_table[automata_state]:
        if match((ks.keycode, ks.keystate), opt[0]):
            automata_state = opt[1]
            operations = opt[2:]
            break

    for op in operations:
        op(ks)

    ks.reset()

def main():
    global dev
    global ui

    ks = keystroke()

    dev.grab()

    for event in dev.read_loop():
        kev = evdev.categorize(event)
        ks.input(kev)

        process(ks)

    dev.ungrab()

    dev.close()
    ui.close()

if __name__ == "__main__":
    main()

