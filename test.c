#include <stdio.h>
#include <stdlib.h>
#include <ffi.h>
#include <gmp.h>

void print_mpz(mpz_t a)
{
    // Initialize the GMP string
    char *str = mpz_get_str(NULL, 10, a);
    printf("%s\n", str);
}

int main()
{
    // Initialize FFI
    ffi_cif cif;
    ffi_type *args[1];
    void *values[1];

    mpz_t *num = malloc(sizeof(mpz_t));
    mpz_init_set_si(*num, 10);

    ffi_type *elements[2];
    elements[0] = &ffi_type_pointer;
    elements[1] = NULL;

    ffi_type ffi_type_mpz;
    ffi_type_mpz.size = ffi_type_mpz.alignment = 0;
    ffi_type_mpz.type = FFI_TYPE_STRUCT;
    ffi_type_mpz.elements = elements;

    // Set argument types and values
    args[0] = &ffi_type_mpz;
    values[0] = num;

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
