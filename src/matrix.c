/**
 * \file matrix.c
 * \brief Contains functions for matrix multiplikation
 */

/**
 * \brief Contains a matrix
 */
typedef struct {
  double *data;      /**< Array where the float numbers are stored */
  int rows;          /**< Number of rows in the matrix */
  int cols;          /**< Number of columns in the matrix */
} Matrix;

/**
 * \brief Allocate memory for a matrix
 */
Matrix* matrix_malloc(int rows, int cols){
  Matrix *m = (Matrix*) malloc(sizeof(Matrix));
  m->rows = rows;
  m->cols = cols;
  m->data = (double*) malloc(sizeof(double[rows][cols]));
  return m;
}

/**
 * \brief Init matrix with defined values from an array
 * Warning: Memory error – array length is not checked!
 */
Matrix* matrix_init(int rows, int cols, const double values[]){
  Matrix *m = matrix_malloc(rows, cols);
  for(int i=0; i<rows*cols; i++){
    m->data[i] = values[i];
  }
  return m;
}

/**
 * \brief Init matrix with zeros
 */
Matrix* matrix_zeros(int rows, int cols){
  Matrix *m = matrix_malloc(rows, cols);
  for(int i=0; i<rows*cols; i++) m->data[i] = 0;
  return m;
}

/**
 * \brief Init matrix with ones
 */
Matrix* matrix_ones(int rows, int cols){
  Matrix *m = matrix_malloc(rows, cols);
  for(int i=0; i<rows*cols; i++) m->data[i] = 1;
  return m;
}

/**
 * \brief Init identity matrix
 */
Matrix* matrix_identity(int dim){
  Matrix *m = matrix_malloc(dim, dim);
  for(int i=0; i<dim; i++){
    for(int j=0; j<dim; j++){
      if(i==j){
        m->data[i + j*dim] = 1;
      }else{
        m->data[i + j*dim] = 0;
      }
    }
  }
  return m;
}

/**
 * \brief Set value in a given matrix
 */
void matrix_set(Matrix* m, int row, int col, double value){
  if(row < m->rows && col < m->cols){
    m->data[col + (row * m->cols)] = value;
  }else{
    fprintf(stderr, "Error: Set matrix out of bounds\n");
    exit(1);
  }
}

/**
 * \brief Free the memory of the matrix
 */
void matrix_free(Matrix* m){
  free(m->data);
  free(m);
}

/**
 * \brief Get element of the matrix
 */
double matrix_get(Matrix* m, int row, int col){
  if(row < m->rows && col < m->cols)
    return m->data[col + (row * m->cols)];
  else
    return 0;
}

/**
 * \brief Display the matrix
 */
void matrix_display(Matrix* m, char *name){
  int i, j;
  printf("%s (%d rows, %d cols)\n", name, m->rows, m->cols);
  for(i = 0; i < m->rows; i++){
    printf(" |");
    for(j = 0; j < m->cols; j++){
      printf(" %.2f ", matrix_get(m, i, j));
    }
    printf("|\n");
  }
}

/**
 * \brief Matrix multiplication
 */
Matrix* matrix_multiply(Matrix *a, Matrix *b){
  /* first check dimensions */
  if(a->cols != b->rows){
    fprintf(stderr, "matrix multiply: a->cols != b->rows, shapes not aligned\n");
    exit(1);
  }
  /* create result matrix */
  Matrix *c = matrix_malloc(a->rows, b->cols);
  /* Multiply */
  for(int i = 0; i < a->rows; i++){
    for(int j = 0; j < b->cols; j++){
      double tmp = 0;
      for(int k = 0; k < a->cols; k++){
        tmp += a->data[i * a->cols + k] * b->data[k * b->cols + j];
      }
      matrix_set(c, i, j, tmp);
    }
  }
  return c;
}

