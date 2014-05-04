from distutils.core import setup, Extension
from os.path import split, splitext
import sys

EXE = splitext(split(sys.executable)[1])[0]
DEBUG = EXE[-2:] == "_d"
LIBPREFIX = "python{}{}".format(*sys.version_info[:2])
LIBNAME = LIBPREFIX + "_d" if DEBUG else LIBPREFIX
NAME = "tester"

tester = Extension(NAME,
                   sources=["testmodule.c"],
                   libraries=[LIBNAME],
                   extra_compile_args=["/Zi"],  # Generate PDBs
                   extra_link_args=["/MANIFEST", "/DEBUG",
                     "/PDB:{}.pdb".format(NAME + "_d" if DEBUG else NAME)]
                  )


setup(name="tester",
      version="0.1.0",
      description="Test extension that crashes and does bad stuff",
      maintainer="Brian Curtin",
      maintainer_email="brian@python.org",
      ext_modules=[tester],
     )

