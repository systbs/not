#include <stdio.h>
#include <stdlib.h>
#include <ffi.h>
#include <gmp.h>

void print_mpz(mpz_t *a, mpz_t *b)
{
    // Initialize the GMP string
    char *str = mpz_get_str(NULL, 10, *a);
    printf("%s\n", str);
}

int main()
{
    // Initialize FFI
    ffi_cif cif;
    ffi_type *args[2];
    void *values[2];

    mpz_t **s = malloc(sizeof(mpz_t *));

    mpz_t *num = malloc(sizeof(mpz_t));
    mpz_init_set_si(*num, 10);

    s[0] = num;

    void *ptr = s;

    // Set argument types and values
    args[0] = &ffi_type_pointer;
    values[0] = ptr;

    args[1] = &ffi_type_pointer;
    values[1] = ptr;

    // Prepare the ffi_cif structure
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1, &ffi_type_void, args) == FFI_OK)
    {
        // Call the function using FFI
        ffi_call(&cif, FFI_FN(&print_mpz), NULL, values);
    }

    // Clean up
    // mpz_clear(num);
    return 0;
}
