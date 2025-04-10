#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_c.c"

/* 함수명과 반환형을 출력하는 함수 */
void getFunctionSignature(json_value *json) {
    json_value ext = json_get(*json, "ext");
    int ext_len = json_len(ext);
    
    printf("- 함수 목록 -\n");
    
    for (int i = 0; i < ext_len; i++) {
        json_value node = json_get(ext, i);
        char* node_type = json_get_string(node, "_nodetype");
        
        // 함수 정의(FuncDef) 처리
        if (strcmp(node_type, "FuncDef") == 0) {
            json_value decl = json_get(node, "decl");
            char* func_name = json_get_string(decl, "name");
            
            // 타입 정보 가져오기
            json_value type = json_get(decl, "type");
            json_value func_type = json_get(type, "type");
            
            // 반환 타입 가져오기
            json_value return_type_node = json_get(func_type, "type");
            char* return_type_nodetype = json_get_string(return_type_node, "_nodetype");
            
            char* return_type = NULL;
            
            if (strcmp(return_type_nodetype, "IdentifierType") == 0) {
                // 기본 타입인 경우
                json_value names = json_get(return_type_node, "names");
                return_type = json_get_string(names, 0);
            } else if (strcmp(return_type_nodetype, "TypeDecl") == 0) {
                // TypeDecl인 경우 한 단계 더 들어가기
                json_value identifier_type = json_get(return_type_node, "type");
                json_value names = json_get(identifier_type, "names");
                return_type = json_get_string(names, 0);
            } else if (strcmp(return_type_nodetype, "PtrDecl") == 0) {
                // 포인터 타입인 경우
                json_value ptr_type = json_get(return_type_node, "type");
                json_value ptr_type_decl = json_get(ptr_type, "type");
                json_value names = json_get(ptr_type_decl, "names");
                
                // 포인터 표시 추가
                char* base_type = json_get_string(names, 0);
                char* ptr_return_type = malloc(strlen(base_type) + 3);
                sprintf(ptr_return_type, "%s*", base_type);
                return_type = ptr_return_type;
            }
            
            printf("함수 이름: %-15s | 반환형: %s\n", func_name, return_type ? return_type : "unknown");
            
            // 포인터 타입인 경우 메모리 해제
            if (return_type_nodetype && strcmp(return_type_nodetype, "PtrDecl") == 0) {
                free(return_type);
            }
        }
        // 함수 선언(Decl) 처리
        else if (strcmp(node_type, "Decl") == 0) {
            // 타입 정보 가져오기
            json_value type = json_get(node, "type");
            char* type_nodetype = json_get_string(type, "_nodetype");
            
            // FuncDecl인 경우만 처리
            if (strcmp(type_nodetype, "FuncDecl") == 0) {
                char* func_name = json_get_string(node, "name");
                
                // 반환 타입 가져오기
                json_value func_type = json_get(type, "type");
                char* func_type_nodetype = json_get_string(func_type, "_nodetype");
                
                char* return_type = NULL;
                
                if (strcmp(func_type_nodetype, "TypeDecl") == 0) {
                    json_value identifier_type = json_get(func_type, "type");
                    json_value names = json_get(identifier_type, "names");
                    return_type = json_get_string(names, 0);
                } else if (strcmp(func_type_nodetype, "PtrDecl") == 0) {
                    json_value ptr_type = json_get(func_type, "type");
                    json_value identifier_type = json_get(ptr_type, "type");
                    json_value names = json_get(identifier_type, "names");
                    
                    // 포인터 표시 추가
                    char* base_type = json_get_string(names, 0);
                    char* ptr_return_type = malloc(strlen(base_type) + 3);
                    sprintf(ptr_return_type, "%s*", base_type);
                    return_type = ptr_return_type;
                }
                
                printf("함수 이름: %-15s | 반환형: %s\n", func_name, return_type ? return_type : "unknown");
                
                // 포인터 타입인 경우 메모리 해제
                if (func_type_nodetype && strcmp(func_type_nodetype, "PtrDecl") == 0) {
                    free(return_type);
                }
            }
        }
    }
    
    printf("----------------------------------------\n");
}

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
    getFunctionSignature(&json);

    // for(int i=0; i<json_len(json2); i++) {
    //     json_value json3 = json_get(json2, i);
    //     json_value json4 = json_get(json3, "_nodetype");
    //     json_print(json4); putchar('\n');
    // }
        

    json_free(json);

    return 0;
}