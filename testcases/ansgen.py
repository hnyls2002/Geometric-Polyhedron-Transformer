import os

filelist = []
for root, dirs, files in os.walk('.'):
    for filename in files:
        if filename.endswith('.clay.scop'):
            filelist.append(os.path.join(root, filename))

os.system('g++ ../src/codegen.cpp -lcloog-isl -lclan -losl -o ./codegen')

for filename in filelist:
    os.system('./codegen ' + filename + ' ' + filename[:-5] + '.clay.c')

os.system('rm codegen')

print(filelist)