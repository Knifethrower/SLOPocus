/*
 * Copyright (C) 2023, A. Roldán. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdint.h>

//#define LEAKED_BETA
//#define SHAREWARE_1_0
//#define SHAREWARE_1_1
//#define REGISTERED_1_0
#define REGISTERED_1_1

#if defined(LEAKED_BETA)
#   define NUMBER_FILES     236
#   define OFFSET           0x01AD74
#elif defined(SHAREWARE_1_0)
#   define NUMBER_FILES     252
#   define OFFSET           0x01EE04
#elif defined(SHAREWARE_1_1)
#   define NUMBER_FILES     253
#   define OFFSET           0x01F0E4
#elif defined(REGISTERED_1_0)
#   define NUMBER_FILES     651
#   define OFFSET           0x01EEB4
#elif defined(REGISTERED_1_1)
#   define NUMBER_FILES     652
#   define OFFSET           0x01F1A4
#else
#   error("Error: version not valid.")
#endif

struct _fat_entry {
    uint32_t offset;
    uint32_t size;
};

struct _fat {
    struct _fat_entry entries[NUMBER_FILES];
};

int main() {
    FILE *file;
    int i = 0;
    struct _fat fat;

    printf("Hocus Pocus FAT extractor\n");
    printf("Number of files: %d\n", NUMBER_FILES);

    file = fopen("HOCUS.EXE", "rb");
    fseek(file, OFFSET, SEEK_SET);
    for (i = 0; i < NUMBER_FILES; i++) {
        fread((void*)&fat.entries[i], sizeof(struct _fat_entry), 1, file);
    }

    fclose(file);

    const uint32_t n = NUMBER_FILES;
    file = fopen("HOCUS.FAT", "wb");
    fwrite((void*)&n, 4, 1, file);
    for (i = 0; i < NUMBER_FILES; i++) {
        fwrite((void*)&fat.entries[i], sizeof(struct _fat_entry), 1, file);
    }
    fclose(file);

    return 0;
}
