# Do a bunch of crashes, but focus more on the library options.

""" TODO: Do something better than time.sleep to wait for the crash dumps to
          be written. time.sleep(1) isn't horrible, but if we have to go above
          that, then move to what we did for os.kill and poll the process or
          something like that.
"""

import unittest
import tempfile
import subprocess
import shutil
import sys
import os
import time
import re

from minidumper import (MiniDumpWithFullMemoryInfo,
                        MiniDumpWithThreadInfo) # Randomly chosen

# Note: There's no explicit test for the `dir` option of `enable` since we
#       always use it for each test anyway.

class EnableOptions(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp(dir=os.getcwd())

    def tearDown(self):
        shutil.rmtree(self.dir)

    def test_custom_name(self):
        subprocess.Popen([sys.executable, "-c",
                          "import minidumper, tester;"
                          "minidumper.enable(name='custom', dir=r'{}');"
                          "tester.access_violation()".format(self.dir)]).wait()

        self.assertIn("custom", [x[:6] for x in os.listdir(self.dir)])

    def test_mdmp_size(self):
        for type in (MiniDumpWithFullMemoryInfo, MiniDumpWithThreadInfo): 
            subprocess.Popen([sys.executable, "-c",
                              "import minidumper, tester;"
                              "minidumper.enable(type={}, dir=r'{}');"
                              "tester.access_violation()".format(
                                                    type, self.dir)]).wait()


        contents = os.listdir(self.dir)
        self.assertEqual(2, len(contents))
        get_size = lambda f: os.stat(os.path.join(self.dir, f)).st_size
        # All we're doing is testin file size. For the two types chosen,
        # they really shouldn't be the same, although there's obviously no
        # formula, and I guess it still depends on the particular crash.
        self.assertNotEqual(get_size(contents[0]), get_size(contents[1]))

    def test_double_enable(self):
        # Double initialization should work fine.
        # This used to not be the case, so it raised a RuntimeError...
        proc = subprocess.Popen([sys.executable, "-c",
                          "import minidumper, tester;"
                          "minidumper.enable(dir=r'{0}');"
                          "minidumper.enable(dir=r'{0}');".format(self.dir)],
                                stderr=subprocess.PIPE)
        stderr = proc.stderr.read().decode(sys.getfilesystemencoding())
        self.addCleanup(proc.stderr.close)
        text = re.sub("^\[\d* refs\]\\n$", "", stderr)
        self.assertEqual("", text)


if __name__ == "__main__":
    unittest.main()

