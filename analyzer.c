#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_c.c"



/* 함수의 개수를 센 후 반환하는 함수 */
int function_count(json_value *json) {
    json_value ext = json_get(*json, "ext");
    int count = 0;

    for (int i = 0; i < json_len(ext); i++)
    {
        json_value object = json_get(ext, i);
        json_value _nodetype = json_get(object, "_nodetype");
        char *_nodetypeString = json_to_string(_nodetype);
        
        if(strcmp(_nodetypeString, "FuncDef") == 0) count++;
    }

    return count;    
}

/* 동적 할당 & 메모리에 파일 내용을 쓰는 함수 */
char *read_file_to_buffer(FILE *fp) {
    fseek(fp, 0, SEEK_END); // File Pointer를 File의 끝으로 설정
    long file_size = ftell(fp); // File Pointer의 위치를 저장(파일의 길이)
    fseek(fp, 0, SEEK_SET); // File Pointer를 File의 시작으로 설정
    char *file_contents = malloc(file_size + 1); // 파일의 길이 + 1 만큼 동적 메모리 할당

    /* 메모리가 정상적으로 할당되었는지 확인 */
    if (file_contents == NULL) {
        perror("Memory allocation failed");
        fclose(fp);
        return 1;
    }
    
    fread(file_contents, 1, file_size, fp); // file_contents를 파일 내용으로 초기화
    file_contents[file_size] = '\0'; // 문자열의 끝에 NULL Byte 추가

    fclose(fp); // file 닫기.

    return file_contents; // 문자열 반환
}

int main() {
    /* ast.json 파일을 Read Only 모드로 열기 */
    FILE *fp;
    fp = fopen("ast.json", "r");

    /* 파일이 잘 열렸는지 확인 */ 
    if(fp == NULL) {
        printf("파일 열기 실패\n");
    } else {
        printf("파일 열기 성공\n");
    }
    
    char *file_contents = read_file_to_buffer(fp); // 파일의 내용을 string으로 변환
    json_value json = json_create(file_contents); // string을 json_value로 변환
    free(file_contents);
    int count = function_count(&json);
    printf("count = %d\n", count);

    // for(int i=0; i<json_len(json2); i++) {
    //     json_value json3 = json_get(json2, i);
    //     json_value json4 = json_get(json3, "_nodetype");
    //     json_print(json4); putchar('\n');
    // }
        

    json_free(json);

    return 0;
}