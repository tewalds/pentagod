
from subprocess import Popen, PIPE
import random

class GTPClient(object):
    def __init__(self, cmdline, params=[]):
        self._proc = Popen(cmdline, stdin=PIPE, stdout=PIPE)
        for p in params:
            self.cmd(p)

    def close(self):
        self.cmd("quit")
        self._proc.terminate()
        self._proc.wait()

    def name(self):
        return self.cmd("name")

    def time(self, game, move):
        self.cmd("time -g %s -m %s" % (game, move))

    def play(self, side, m):
        self.cmd("play %s %s" % (side, m))

    def genmove(self, side):
        return self.cmd("genmove %s" % (side, ))

    def cmd(self, c):
        print ">", c.strip()
        self._proc.stdin.write(c.strip() + "\n")
        result = ""
        while True:
            line = self._proc.stdout.readline()
            result += line
            if line == "\n":
                if result[0] != '=':
                    raise Exception(result)
                print result.strip()
                return result[2:].strip()

