import os
import sys

version = str(sys.argv[1])
in_dev = str(sys.argv[2])
 
os.system(f'echo "building { version } core dll..." && cd ../../src/core && make dynamic BUILDTYPE="-shared" BUILDFLAG="-o" DEVELOPMENT="{ in_dev }" USE_JSON="1" STANDALONE="1" THREAD_COUNT="10" OUTFILE="../../build/sdk/spaghyeti-{ version }.dll"')

 