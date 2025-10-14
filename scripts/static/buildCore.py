import os
import sys

make = 'web' if int(sys.argv[1]) == 1 else 'static BUILDTYPE="-static" BUILDFLAG="-c" DEVELOPMENT="0" USE_JSON="1" STANDALONE="1" THREAD_COUNT="33"'
lib = 'spaghyeti-web.a' if int(sys.argv[1]) == 1 else 'spaghyeti.a'
build_type = 'web' if int(sys.argv[1]) == 1 else 'standard'

# tmp_dir = '../../src/core/tmp'

# if not os.path.exists(tmp_dir):
#     os.mkdir(tmp_dir)

os.system(f'echo build static core ({ build_type })... && cd ../../src/core && make { make } && mv { lib } ../../build/sdk && rm -f *.o') 