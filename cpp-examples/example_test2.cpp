#include <iostream>
#include <cstdint>

const uint64_t KEY = 0x123456789abcdef0;

// 非线性变换函数
uint32_t nonlinear_transform(uint32_t x)
{
	// 置换操作
	x = ((x & 0x0000ffff) << 16) | ((x & 0xffff0000) >> 16);
	x = ((x & 0x00ff00ff) << 8) | ((x & 0xff00ff00) >> 8);
	x = ((x & 0x0f0f0f0f) << 4) | ((x & 0xf0f0f0f0) >> 4);
	x = ((x & 0x33333333) << 2) | ((x & 0xcccccccc) >> 2);
	x = ((x & 0x55555555) << 1) | ((x & 0xaaaaaaaa) >> 1);

	// 替换操作
	x = ((x << 1) & 0xfffffffe) ^ ((x >> 31) & 0x00000001);
	x = ((x << 3) & 0xfffffff8) ^ ((x >> 29) & 0x00000007);
	x = ((x << 5) & 0xffffffe0) ^ ((x >> 27) & 0x0000001f);
	x = ((x << 7) & 0xffffff80) ^ ((x >> 25) & 0x0000007f);
	x = ((x << 11) & 0xfffff800) ^ ((x >> 21) & 0x000007ff);
	x = ((x << 13) & 0xffffe000) ^ ((x >> 19) & 0x00001fff);
	x = ((x << 17) & 0xfffe0000) ^ ((x >> 15) & 0x0003ffff);
	x = ((x << 19) & 0xfff00000) ^ ((x >> 13) & 0x000fffff);

	return x;
}

// 加密函数
uint64_t encrypt(uint64_t plaintext)
{
	uint32_t left = plaintext >> 32;
	uint32_t right = plaintext & 0xffffffff;

	for (int i = 0; i < 16; i++)
	{
		uint32_t temp = left;
		left = right ^ KEY;
		right = temp ^ nonlinear_transform(right);
		std::swap(left, right);
	}

	return ((uint64_t) left << 32) | right;
}

// 解密函数
uint64_t decrypt(uint64_t ciphertext)
{
	uint32_t left = ciphertext >> 32;
	uint32_t right = ciphertext & 0xffffffff;

	for (int i = 0; i < 16; i++)
	{
		uint32_t temp = right;
		right = left ^ KEY;
		left = temp ^ nonlinear_transform(left);
		std::swap(left, right);
	}

	return ((uint64_t) left << 32) | right;
}

int main()
{
	uint64_t plaintext = 1688855393460119;
	uint64_t ciphertext = encrypt(plaintext);
	uint64_t decrypted = decrypt(ciphertext);

	std::cout << "Plaintext: " << plaintext << std::endl;
	std::cout << "Ciphertext: " << ciphertext << std::endl;
	std::cout << "Decrypted: " << decrypted << std::endl;

	return 0;
}