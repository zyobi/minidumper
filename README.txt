`minidumper` is a C extension for writing "minidumps" for post-mortem
analysis of crashes in Python or its extensions.::

   import minidumper 
   minidumper.enable(name="myapp", dir="data", type=minidumper.MiniDumpWithFullMemory)
   # Run your app, do stuff that crashes.
   # Look for `data/myapp_20110929-050934.mdmp`
   minidumper.disable() # Unhook crash handler, go back to normal.

There really isn't that much to it. It's probably of limited utility unless
you're an extension developer and you're working with a debug build of
Python, as release builds don't carry the same helpful information.

The project is very much "in progress".

