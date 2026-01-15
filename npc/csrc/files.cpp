#include "npc.h"
#include <cstdint>

static void initMem() {
    M = (vluint32_t*)calloc(MEM_SIZE, sizeof(vluint32_t));
    if (!M) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
}
uint32_t loadFile(int argc, char** argv) {
	initMem();
	size_t readWords = 0;
	if (argc < 3) {
		printf(COLOR_RED "Missing file path!\n" COLOR_NONE);
		for(int i = 0; i < argc; i++) {printf("%s", argv[i]);}
		exit(1);
	}
	FILE *file = fopen(argv[2], "rb");
	if (file) {
		// 计算文件大小
		fseek(file, 0, SEEK_END);
		long fileSize = ftell(file);
		rewind(file);
		
		// 确保不会超出M数组的大小
		size_t wordsToRead = fileSize / sizeof(uint32_t);
		if (wordsToRead > MEM_SIZE) {
			printf("文件过大，无法加载到内存中,你需要%zu\n", wordsToRead);
			exit(1);
		}
		
		// 读取数据到M数组
		readWords = fread(M, sizeof(uint32_t), wordsToRead, file);
		fclose(file);
		
		printf(COLOR_GREEN "成功加载 %ld 字节 (%zu 个32位字) 到内存\n" COLOR_NONE , readWords * sizeof(uint32_t), readWords);
	} else {
		printf(COLOR_RED "无法打开文件: %s\n" COLOR_NONE , argv[2]);
		exit(1);
	}
	return (readWords * sizeof(uint32_t));
}