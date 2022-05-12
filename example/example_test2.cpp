#include <iostream>
#include <utility>
#include <cstring>

typedef struct {
    float *data;
    size_t nrows;
    size_t ncols;
} matrix;

enum matrix_err_code {
    MATRIX_SUCCESS,
    MATRIX_ERR_MEMORY_INSUFFICIENT,
    MATRIX_ERR_MISMATCHED_MATRIX_SIZE,
//    …
};

int matrix_alloc(matrix *ptr,
                 size_t nrows,
                 size_t ncols) {
    size_t size = nrows * ncols * sizeof(float);
    float *data = malloc(size);
    if (data == NULL) {
        return MATRIX_ERR_MEMORY_INSUFFICIENT;
    }
    ptr->data = data;
    ptr->nrows = nrows;
    ptr->ncols = ncols;
}

void matrix_dealloc(matrix *ptr) {
    if (ptr->data == NULL) {
        return;
    }
    free(ptr->data);
    ptr->data = NULL;
    ptr->nrows = 0;
    ptr->ncols = 0;
}


int matrix_multiply(matrix *result,
                    const matrix *lhs,
                    const matrix *rhs) {
    int errcode;
    if (lhs->ncols != rhs->nrows) {
        return MATRIX_ERR_MISMATCHED_MATRIX_SIZE;
        // 呃，得把这个错误码添到 enum matrix_err_code 里
    }
    errcode = matrix_alloc(result, lhs->nrows, rhs->ncols);
    if (errcode != MATRIX_SUCCESS) {
        return errcode;
    }
    // 进行矩阵乘法运算
    return MATRIX_SUCCESS;
}

int main() {
    matrix c;

    // 不清零的话，错误处理和资源清理会更复杂
    memset(&c, 0, sizeof(matrix));

    int errcode = matrix_multiply(c, a, b);
    if (errcode != MATRIX_SUCCESS) {
        goto error_exit;
    }
    // 使用乘法的结果做其他处理

error_exit:
    matrix_dealloc(&c);
    return errcode;
}