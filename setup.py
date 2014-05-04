from distutils.core import setup, Extension
from os.path import split, splitext
import sys

EXE = splitext(split(sys.executable)[1])[0]
DEBUG = EXE[-2:] == "_d"
LIBPREFIX = "python{}{}".format(*sys.version_info[:2])
LIBNAME = LIBPREFIX + "_d" if DEBUG else LIBPREFIX

minidumper = Extension("minidumper",
                       sources=["minidumper.c"],
                       libraries=["Dbghelp", LIBNAME],
                       extra_compile_args=["/W4", # Highest warning level
                                           "/Zi"  # Generate PDBs
                                          ],
                       extra_link_args=["/MANIFEST"]
                      )


setup(name="minidumper",
      version="0.1.0",
      description="Windows MiniDump handler/writer",
      maintainer="Brian Curtin",
      maintainer_email="brian@python.org",
      ext_modules=[minidumper],
     )

