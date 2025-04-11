#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_c.c"

/* 함수의 반환형(Return Type)을 출력해주는 함수 */
void get_return_type(json_value *ast) {
    json_value ext = json_get(*ast, "ext"); // ext 배열열
    int ext_len = json_len(ext); // ext 배열의 크기

    // 배열의 크기만큼 Loop를 돎
    for(int i = 0; i < ext_len; i++) {
        json_value node = json_get(ext, i); // 배열의 i번째 노드 저장
        char* _nodetype = json_get_string(node, "_nodetype"); // 노드 타입 문자열로 저장

        if(strcmp(_nodetype, "FuncDef") == 0) {
            // 노드 타입이 함수 정의일 때만 실행
            json_value decl_node = json_get(node, "decl"); // FunDef 노드에서 decl 노드 저장
            char* func_name = json_get_string(decl_node, "name"); // decl 노드에서 함수 이름 문자열로 추출
            char* ret_type;
            printf("[%d] %s ", i, func_name); // 함수 이름 출력

            json_value type_1 = json_get(decl_node, "type");
            json_value type_2 = json_get(type_1, "type");
            json_value type_3 = json_get(type_2, "type");

            char* type_2_nodetype = json_get_string(type_2, "_nodetype");
            if(strcmp(type_2_nodetype, "PtrDecl") == 0) {
                // 반환형이 포인터인 경우
                json_value type_4 = json_get(type_3, "type");
                ret_type = json_get_string(type_4, "names", 0);
                printf("| *%s\n", ret_type);
            } else {
                // 반환형이 포인터가 아닌 경우
                ret_type = json_get_string(type_3, "names", 0);
                printf("| %s\n", ret_type);
            }           
            
        }
    }
}

/* 함수의 개수를 출력해주는 함수 */
int count_function(json_value *ast) {
    json_value ext = json_get(*ast, "ext"); // ext 배열
    int ext_len = json_len(ext); // ext 배열의 크기
    int func_count = 0; // 함수 개수 카운트 변수

    // 배열의 크기만큼 Loop를 돎
    for(int i = 0; i < ext_len; i++) {
        json_value node = json_get(ext, i); // 배열의 i번째 노드 저장
        char* _nodetype = json_get_string(node, "_nodetype"); // 노드 타입 문자열로 저장
        // 노드 타입이 "FuncDef"일 경우 카운트 증가
        if(strcmp(_nodetype, "FuncDef") == 0) func_count++; 
    }

    return func_count; // 함수 개수 반환
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

    // 함수 개수 출력
    int func_count = count_function(&json);
    printf("func_count = %d\n", func_count);

    get_return_type(&json);

    json_free(json);

    return 0;
}