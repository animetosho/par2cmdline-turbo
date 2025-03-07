
#include "../src/hedley.h"

// basic
#define FUNCS(v) \
	void gf16_shuffle_prepare_packed_cksum_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t srcLen, size_t sliceLen, unsigned inputPackSize, unsigned inputNum, size_t chunkLen); \
	void gf16_shuffle_prepare_partial_packsum_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t srcLen, size_t sliceLen, unsigned inputPackSize, unsigned inputNum, size_t chunkLen, size_t partOffset, size_t partLen); \
	int gf16_shuffle_finish_packed_cksum_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t sliceLen, unsigned numOutputs, unsigned outputNum, size_t chunkLen); \
	int gf16_shuffle_finish_partial_packsum_##v(void *HEDLEY_RESTRICT dst, void *HEDLEY_RESTRICT src, size_t sliceLen, unsigned numOutputs, unsigned outputNum, size_t chunkLen, size_t partOffset, size_t partLen); \
	void gf16_shuffle_muladd_##v(const void *HEDLEY_RESTRICT scratch, void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t len, uint16_t coefficient, void *HEDLEY_RESTRICT mutScratch); \
	void gf16_shuffle_muladd_prefetch_##v(const void *HEDLEY_RESTRICT scratch, void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t len, uint16_t coefficient, void *HEDLEY_RESTRICT mutScratch, const void *HEDLEY_RESTRICT prefetch); \
	extern int gf16_shuffle_available_##v

FUNCS(ssse3);
FUNCS(avx);
FUNCS(avx2);
FUNCS(avx512);

#undef FUNCS

#ifdef PARPAR_INVERT_SUPPORT
#define FUNCS(v) \
	void gf16_shuffle_prepare_##v(void* dst, const void* src, size_t srcLen); \
	void gf16_shuffle_finish_##v(void *HEDLEY_RESTRICT dst, size_t len); \
	void gf16_shuffle_mul_##v(const void *HEDLEY_RESTRICT scratch, void* dst, const void* src, size_t len, uint16_t coefficient, void *HEDLEY_RESTRICT mutScratch)
FUNCS(ssse3);
FUNCS(avx);
FUNCS(avx2);
FUNCS(avx512);
#undef FUNCS
#endif

#ifdef PARPAR_INCLUDE_BASIC_OPS
#define FUNCS(v) \
	void gf16_shuffle_prepare_packed_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t srcLen, size_t sliceLen, unsigned inputPackSize, unsigned inputNum, size_t chunkLen); \
	void gf16_shuffle_finish_packed_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t sliceLen, unsigned numOutputs, unsigned outputNum, size_t chunkLen)
FUNCS(ssse3);
FUNCS(avx);
FUNCS(avx2);
FUNCS(avx512);
#undef FUNCS
#endif

// multi-region
#define FUNCS(v) \
	void gf16_shuffle_muladd_multi_packed_##v(const void *HEDLEY_RESTRICT scratch, unsigned packRegions, unsigned regions, void *HEDLEY_RESTRICT dst, const void* HEDLEY_RESTRICT src, size_t len, const uint16_t *HEDLEY_RESTRICT coefficients, void *HEDLEY_RESTRICT mutScratch); \
	void gf16_shuffle_muladd_multi_packpf_##v(const void *HEDLEY_RESTRICT scratch, unsigned packRegions, unsigned regions, void *HEDLEY_RESTRICT dst, const void* HEDLEY_RESTRICT src, size_t len, const uint16_t *HEDLEY_RESTRICT coefficients, void *HEDLEY_RESTRICT mutScratch, const void* HEDLEY_RESTRICT prefetchIn, const void* HEDLEY_RESTRICT prefetchOut)

FUNCS(avx512);
FUNCS(vbmi);
FUNCS(neon);
FUNCS(128_sve);
FUNCS(128_sve2);
FUNCS(512_sve2);
FUNCS(128_rvv);

#undef FUNCS

#ifdef PARPAR_INVERT_SUPPORT
#define FUNCS(v) \
	void gf16_shuffle_muladd_multi_##v(const void *HEDLEY_RESTRICT scratch, unsigned regions, size_t offset, void *HEDLEY_RESTRICT dst, const void* const*HEDLEY_RESTRICT src, size_t len, const uint16_t *HEDLEY_RESTRICT coefficients, void *HEDLEY_RESTRICT mutScratch); \
	void gf16_shuffle_muladd_multi_stridepf_##v(const void *HEDLEY_RESTRICT scratch, unsigned regions, size_t srcStride, void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t len, const uint16_t *HEDLEY_RESTRICT coefficients, void *HEDLEY_RESTRICT mutScratch, const void* HEDLEY_RESTRICT prefetch)
FUNCS(avx512);
FUNCS(vbmi);
FUNCS(neon);
FUNCS(128_sve);
FUNCS(128_sve2);
FUNCS(512_sve2);
FUNCS(128_rvv);
#undef FUNCS
#endif

void gf16_shuffle_muladd_vbmi(const void *HEDLEY_RESTRICT scratch, void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t len, uint16_t coefficient, void *HEDLEY_RESTRICT mutScratch);
void gf16_shuffle_muladd_prefetch_vbmi(const void *HEDLEY_RESTRICT scratch, void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t len, uint16_t coefficient, void *HEDLEY_RESTRICT mutScratch, const void *HEDLEY_RESTRICT prefetch);
extern int gf16_shuffle_available_vbmi;

#define FUNCS(v) \
	void gf16_shuffle_muladd_##v(const void *HEDLEY_RESTRICT scratch, void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t len, uint16_t coefficient, void *HEDLEY_RESTRICT mutScratch)

FUNCS(neon);
FUNCS(128_sve);
FUNCS(128_sve2);
FUNCS(512_sve2);
FUNCS(128_rvv);

#undef FUNCS

#ifdef PARPAR_INVERT_SUPPORT
#define FUNCS(v) \
	void gf16_shuffle_mul_##v(const void *HEDLEY_RESTRICT scratch, void* dst, const void* src, size_t len, uint16_t coefficient, void *HEDLEY_RESTRICT mutScratch)
FUNCS(neon);
FUNCS(128_sve);
FUNCS(128_sve2);
FUNCS(512_sve2);
FUNCS(128_rvv);
FUNCS(vbmi);
#undef FUNCS
#endif

#ifdef PARPAR_INCLUDE_BASIC_OPS
#define FUNCS(v) \
	void gf16_shuffle_prepare_packed_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t srcLen, size_t sliceLen, unsigned inputPackSize, unsigned inputNum, size_t chunkLen); \
	void gf16_shuffle_prepare_packed_cksum_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t srcLen, size_t sliceLen, unsigned inputPackSize, unsigned inputNum, size_t chunkLen); \
	void gf16_shuffle_prepare_partial_packsum_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t srcLen, size_t sliceLen, unsigned inputPackSize, unsigned inputNum, size_t chunkLen, size_t partOffset, size_t partLen)
#else
#define FUNCS(v) \
	void gf16_shuffle_prepare_packed_cksum_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t srcLen, size_t sliceLen, unsigned inputPackSize, unsigned inputNum, size_t chunkLen); \
	void gf16_shuffle_prepare_partial_packsum_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t srcLen, size_t sliceLen, unsigned inputPackSize, unsigned inputNum, size_t chunkLen, size_t partOffset, size_t partLen)
#endif

FUNCS(vbmi);
FUNCS(neon);
FUNCS(sve);
FUNCS(512_sve2);
FUNCS(rvv);

#undef FUNCS

#define FUNCS(v) \
	int gf16_shuffle_finish_packed_cksum_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t sliceLen, unsigned numOutputs, unsigned outputNum, size_t chunkLen); \
	int gf16_shuffle_finish_partial_packsum_##v(void *HEDLEY_RESTRICT dst, void *HEDLEY_RESTRICT src, size_t sliceLen, unsigned numOutputs, unsigned outputNum, size_t chunkLen, size_t partOffset, size_t partLen)

FUNCS(neon);
FUNCS(sve);
FUNCS(rvv);

#undef FUNCS

#ifdef PARPAR_INCLUDE_BASIC_OPS
#define FUNCS(v) \
	void gf16_shuffle_finish_packed_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t sliceLen, unsigned numOutputs, unsigned outputNum, size_t chunkLen)
FUNCS(neon);
FUNCS(sve);
FUNCS(rvv);
#undef FUNCS
#endif

// also used for clmul, but declared here for convenience
extern int gf16_available_neon;
extern int gf16_available_sve;
extern int gf16_available_sve2;

extern int gf16_available_rvv;

// shuffle2x
#define FUNCS(v) \
	void gf16_shuffle2x_prepare_packed_cksum_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t srcLen, size_t sliceLen, unsigned inputPackSize, unsigned inputNum, size_t chunkLen); \
	void gf16_shuffle2x_prepare_partial_packsum_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t srcLen, size_t sliceLen, unsigned inputPackSize, unsigned inputNum, size_t chunkLen, size_t partOffset, size_t partLen); \
	int gf16_shuffle2x_finish_packed_cksum_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t sliceLen, unsigned numOutputs, unsigned outputNum, size_t chunkLen); \
	int gf16_shuffle2x_finish_partial_packsum_##v(void *HEDLEY_RESTRICT dst, void *HEDLEY_RESTRICT src, size_t sliceLen, unsigned numOutputs, unsigned outputNum, size_t chunkLen, size_t partOffset, size_t partLen); \
	void gf16_shuffle2x_muladd_##v(const void *HEDLEY_RESTRICT scratch, void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t len, uint16_t coefficient, void *HEDLEY_RESTRICT mutScratch); \
	void gf16_shuffle2x_muladd_multi_packed_##v(const void *HEDLEY_RESTRICT scratch, unsigned packRegions, unsigned regions, void *HEDLEY_RESTRICT dst, const void* HEDLEY_RESTRICT src, size_t len, const uint16_t *HEDLEY_RESTRICT coefficients, void *HEDLEY_RESTRICT mutScratch); \
	void gf16_shuffle2x_muladd_multi_packpf_##v(const void *HEDLEY_RESTRICT scratch, unsigned packRegions, unsigned regions, void *HEDLEY_RESTRICT dst, const void* HEDLEY_RESTRICT src, size_t len, const uint16_t *HEDLEY_RESTRICT coefficients, void *HEDLEY_RESTRICT mutScratch, const void* HEDLEY_RESTRICT prefetchIn, const void* HEDLEY_RESTRICT prefetchOut)

FUNCS(avx2);
FUNCS(avx512);

#undef FUNCS

#ifdef PARPAR_INVERT_SUPPORT
#define FUNCS(v) \
	void gf16_shuffle2x_mul_##v(const void *HEDLEY_RESTRICT scratch, void* dst, const void* src, size_t len, uint16_t coefficient, void *HEDLEY_RESTRICT mutScratch); \
	void gf16_shuffle2x_prepare_##v(void* dst, const void* src, size_t srcLen); \
	void gf16_shuffle2x_finish_##v(void *HEDLEY_RESTRICT dst, size_t len); \
	void gf16_shuffle2x_muladd_multi_##v(const void *HEDLEY_RESTRICT scratch, unsigned regions, size_t offset, void *HEDLEY_RESTRICT dst, const void* const*HEDLEY_RESTRICT src, size_t len, const uint16_t *HEDLEY_RESTRICT coefficients, void *HEDLEY_RESTRICT mutScratch); \
	void gf16_shuffle2x_muladd_multi_stridepf_##v(const void *HEDLEY_RESTRICT scratch, unsigned regions, size_t srcStride, void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t len, const uint16_t *HEDLEY_RESTRICT coefficients, void *HEDLEY_RESTRICT mutScratch, const void* HEDLEY_RESTRICT prefetch)
FUNCS(avx2);
FUNCS(avx512);
#undef FUNCS
#endif


#ifdef PARPAR_INCLUDE_BASIC_OPS
#define FUNCS(v) \
	void gf16_shuffle2x_prepare_packed_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t srcLen, size_t sliceLen, unsigned inputPackSize, unsigned inputNum, size_t chunkLen); \
	void gf16_shuffle2x_finish_packed_##v(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t sliceLen, unsigned numOutputs, unsigned outputNum, size_t chunkLen)
FUNCS(avx2);
FUNCS(avx512);
FUNCS(sve);
#undef FUNCS
#endif


void gf16_shuffle2x_prepare_packed_cksum_sve(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t srcLen, size_t sliceLen, unsigned inputPackSize, unsigned inputNum, size_t chunkLen);
void gf16_shuffle2x_prepare_partial_packsum_sve(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t srcLen, size_t sliceLen, unsigned inputPackSize, unsigned inputNum, size_t chunkLen, size_t partOffset, size_t partLen);
int gf16_shuffle2x_finish_packed_cksum_sve(void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t sliceLen, unsigned numOutputs, unsigned outputNum, size_t chunkLen);
int gf16_shuffle2x_finish_partial_packsum_sve(void *HEDLEY_RESTRICT dst, void *HEDLEY_RESTRICT src, size_t sliceLen, unsigned numOutputs, unsigned outputNum, size_t chunkLen, size_t partOffset, size_t partLen);

void gf16_shuffle2x_muladd_128_sve2(const void *HEDLEY_RESTRICT scratch, void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t len, uint16_t coefficient, void *HEDLEY_RESTRICT mutScratch);
void gf16_shuffle2x_muladd_multi_packed_128_sve2(const void *HEDLEY_RESTRICT scratch, unsigned packRegions, unsigned regions, void *HEDLEY_RESTRICT dst, const void* HEDLEY_RESTRICT src, size_t len, const uint16_t *HEDLEY_RESTRICT coefficients, void *HEDLEY_RESTRICT mutScratch);
void gf16_shuffle2x_muladd_multi_packpf_128_sve2(const void *HEDLEY_RESTRICT scratch, unsigned packRegions, unsigned regions, void *HEDLEY_RESTRICT dst, const void* HEDLEY_RESTRICT src, size_t len, const uint16_t *HEDLEY_RESTRICT coefficients, void *HEDLEY_RESTRICT mutScratch, const void* HEDLEY_RESTRICT prefetchIn, const void* HEDLEY_RESTRICT prefetchOut);

#ifdef PARPAR_INVERT_SUPPORT
void gf16_shuffle2x_mul_128_sve2(const void *HEDLEY_RESTRICT scratch, void* dst, const void* src, size_t len, uint16_t coefficient, void *HEDLEY_RESTRICT mutScratch);
void gf16_shuffle2x_muladd_multi_128_sve2(const void *HEDLEY_RESTRICT scratch, unsigned regions, size_t offset, void *HEDLEY_RESTRICT dst, const void* const*HEDLEY_RESTRICT src, size_t len, const uint16_t *HEDLEY_RESTRICT coefficients, void *HEDLEY_RESTRICT mutScratch);
void gf16_shuffle2x_muladd_multi_stridepf_128_sve2(const void *HEDLEY_RESTRICT scratch, unsigned regions, size_t srcStride, void *HEDLEY_RESTRICT dst, const void *HEDLEY_RESTRICT src, size_t len, const uint16_t *HEDLEY_RESTRICT coefficients, void *HEDLEY_RESTRICT mutScratch, const void* HEDLEY_RESTRICT prefetch);
#endif

void* gf16_shuffle_init_x86(int polynomial);
void* gf16_shuffle_init_vbmi(int polynomial);
void* gf16_shuffle_init_arm(int polynomial);
void* gf16_shuffle_init_128_sve(int polynomial);
void* gf16_shuffle_init_512_sve(int polynomial);
void* gf16_shuffle_init_128_rvv(int polynomial);

int gf16_sve_get_size();
int gf16_rvv_get_size();

#ifdef PARPAR_INVERT_SUPPORT
uint16_t gf16_affine2x_replace_word(void* data, size_t index, uint16_t newValue);
uint16_t gf16_shuffle16_replace_word(void* data, size_t index, uint16_t newValue);
uint16_t gf16_shuffle32_replace_word(void* data, size_t index, uint16_t newValue);
uint16_t gf16_shuffle64_replace_word(void* data, size_t index, uint16_t newValue);
uint16_t gf16_shuffle2x16_replace_word(void* data, size_t index, uint16_t newValue);
uint16_t gf16_shuffle2x32_replace_word(void* data, size_t index, uint16_t newValue);
#endif
