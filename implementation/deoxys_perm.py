H_PERMUTATION     = [1,6,11,12,5,10,15,0,9,14,3,4,13,2,7,8]
H_PERMUTATION_INV = [7,0,13,10,11,4,1,14,15,8,5,2,3,12,9,6]

def gen_perm(L):
    P = L[:]
    for i in range(len(L)):
        P[L[i]] = i
    return P

def apply_perm(P, L):
    LL = L[:]
    for i in range(len(L)):
        LL[P[i]] = L[i]
    return LL

L = [i for i in range(16)]

print(L)
L = apply_perm(H_PERMUTATION, L)
print(L)
L = apply_perm(H_PERMUTATION_INV, L)
print(L)

L = [i for i in range(16)]

for i in range(14):
    L = apply_perm(H_PERMUTATION, L)
print(L)
P = gen_perm(L)
print(P)
