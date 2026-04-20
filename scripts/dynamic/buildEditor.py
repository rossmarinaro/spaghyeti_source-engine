import os

os.system('cd ../../src/core && echo "building debug core dll (editor)..." && make BUILDTYPE="-shared" BUILDFLAG="-o" DEVELOPMENT="1" USE_JSON="1" STANDALONE="0" OUTFILE="../../build/spaghyeditor.dll" && cd ../editor && echo "building editor..." && make')


