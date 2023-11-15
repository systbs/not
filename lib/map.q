import ("./array.q", array)

export type (K, V) pair : [K, V];
export type (K, V) map : array{pair{K, V}};