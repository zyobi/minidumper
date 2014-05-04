from os.path import splitext, split
import subprocess
import sys
import unittest
import os

from .test_options import *

EXE = splitext(split(sys.executable)[1])[0]
DEBUG = EXE[-2:] == "_d"

cwd = os.getcwd()
os.chdir(os.path.split(__file__)[0])
args = ["setup.py", "build"]
args += ["--debug"] if DEBUG else []
args += ["install"]
subprocess.Popen([sys.executable] + args).wait()
os.chdir(cwd)

unittest.main()

