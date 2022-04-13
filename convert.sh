#!/bin/bash

git am skyline-conversion/*.patch

grep -rl '// Copyright 2021 yuzu Emulator Project' --exclude-dir={.git,skyline-conversion} | xargs sed -i 's/\/\/ Copyright 2021 yuzu Emulator Project/\/\/ SPDX-License-Identifier: MPL-2.0/g' 
grep -rl '// Licensed under GPLv2 or any later version' --exclude-dir={.git,skyline-conversion} | xargs sed -i 's/\/\/ Licensed under GPLv2 or any later version/\/\/ Copyright © 2021 yuzu Emulator Project (https:\/\/github.com\/yuzu-emu\/yuzu\/)/g' 
grep -rl '// Refer to the license.txt file included.' --exclude-dir={.git,skyline-conversion} | xargs sed -i '\/\/ Refer to the license.txt file included./d' 


for i in `find . -name "*.cpp" -o -name "*.h" -type f`; do
    sed -i -E 's/"shader_recompiler\/([^"]+)"/<shader_compiler\/\1>/g' $i
    sed -i -E 's/"common\/([^"]+)"/<shader_compiler\/common\/\1>/g' $i
done

for i in `grep -rl 'std::ranges' --exclude-dir={.git,skyline-conversion}`; do
    sed -i -E 's/std::ranges/ranges/g' $i
    gawk -i inplace '!found && /include/ { print "#include <range/v3/algorithm.hpp>"; found=1 } 1' $i
done

sed -i 's/ranges::sort(encodings, \[\](const InstEncoding& lhs, const InstEncoding& rhs) {/std::sort(encodings.begin(), encodings.end(), \[\](const InstEncoding& lhs, const InstEncoding& rhs) {/g' frontend/maxwell/decode.cpp


