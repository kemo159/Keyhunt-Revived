BASE_CXXFLAGS = -m64 -mssse3 -Wall -Wextra -Wno-deprecated-copy -Ofast -ftree-vectorize
BASE_CFLAGS = -m64 -mssse3 -Wall -Wextra -Ofast -ftree-vectorize

default:
	g++ $(BASE_CXXFLAGS) -flto -c oldbloom/bloom.cpp -o oldbloom.o
	g++ $(BASE_CXXFLAGS) -flto -c bloom/bloom.cpp -o bloom.o
	gcc $(BASE_CFLAGS) -Wno-unused-parameter -c base58/base58.c -o base58.o
	gcc $(BASE_CFLAGS) -c rmd160/rmd160.c -o rmd160.o
	g++ $(BASE_CXXFLAGS) -c sha3/sha3.c -o sha3.o
	g++ $(BASE_CXXFLAGS) -c sha3/keccak.c -o keccak.o
	gcc $(BASE_CFLAGS) -c xxhash/xxhash.c -o xxhash.o
	g++ $(BASE_CXXFLAGS) -c util.c -o util.o
	g++ $(BASE_CXXFLAGS) -c cpu_features.cpp -o cpu_features.o
	g++ $(BASE_CXXFLAGS) -c secp256k1/Int.cpp -o Int.o
	g++ $(BASE_CXXFLAGS) -c secp256k1/Point.cpp -o Point.o
	g++ $(BASE_CXXFLAGS) -c secp256k1/SECP256K1.cpp -o SECP256K1.o
	g++ $(BASE_CXXFLAGS) -c secp256k1/IntMod.cpp -o IntMod.o
	g++ $(BASE_CXXFLAGS) -flto -c secp256k1/Random.cpp -o Random.o
	g++ $(BASE_CXXFLAGS) -flto -c secp256k1/IntGroup.cpp -o IntGroup.o
	g++ $(BASE_CXXFLAGS) -flto -c hash/ripemd160.cpp -o hash/ripemd160.o
	g++ $(BASE_CXXFLAGS) -flto -c hash/sha256.cpp -o hash/sha256.o
	g++ $(BASE_CXXFLAGS) -flto -c hash/ripemd160_sse.cpp -o hash/ripemd160_sse.o
	g++ $(BASE_CXXFLAGS) -flto -c hash/sha256_sse.cpp -o hash/sha256_sse.o
	g++ $(BASE_CXXFLAGS) -mavx2 -c hash/ripemd160_avx2.cpp -o hash/ripemd160_avx2.o
	g++ $(BASE_CXXFLAGS) -mavx2 -c hash/sha256_avx2.cpp -o hash/sha256_avx2.o
	g++ $(BASE_CXXFLAGS) -o keyhunt keyhunt.cpp base58.o rmd160.o hash/ripemd160.o hash/ripemd160_sse.o hash/ripemd160_avx2.o hash/sha256.o hash/sha256_sse.o hash/sha256_avx2.o bloom.o oldbloom.o xxhash.o util.o cpu_features.o Int.o Point.o SECP256K1.o IntMod.o Random.o IntGroup.o sha3.o keccak.o -lm -lpthread
	rm -r *.o
clean:
	rm keyhunt
legacy:
	g++ -march=native -mtune=native -Wall -Wextra -Ofast -ftree-vectorize -flto -c oldbloom/bloom.cpp -o oldbloom.o
	g++ -march=native -mtune=native -Wall -Wextra -Ofast -ftree-vectorize -flto -c bloom/bloom.cpp -o bloom.o
	gcc -march=native -mtune=native -Wno-unused-result -Ofast -ftree-vectorize -c base58/base58.c -o base58.o
	gcc -march=native -mtune=native -Wall -Wextra -Ofast -ftree-vectorize -c xxhash/xxhash.c -o xxhash.o
	g++ -march=native -mtune=native -Wall -Wextra -Ofast -ftree-vectorize -c util.c -o util.o
	g++ -march=native -mtune=native -Wall -Wextra -Ofast -ftree-vectorize -c sha3/sha3.c -o sha3.o
	g++ -march=native -mtune=native -Wall -Wextra -Ofast -ftree-vectorize -c sha3/keccak.c -o keccak.o
	g++ -march=native -mtune=native -Wall -Wextra -Ofast -ftree-vectorize -c hashing.c -o hashing.o
	g++ -march=native -mtune=native -Wall -Wextra -Ofast -ftree-vectorize -c gmp256k1/Int.cpp -o Int.o
	g++ -march=native -mtune=native -Wall -Wextra -Ofast -ftree-vectorize -c gmp256k1/Point.cpp -o Point.o
	g++ -march=native -mtune=native -Wall -Wextra -Ofast -ftree-vectorize -c gmp256k1/GMP256K1.cpp -o GMP256K1.o
	g++ -march=native -mtune=native -Wall -Wextra -Ofast -ftree-vectorize -c gmp256k1/IntMod.cpp -o IntMod.o
	g++ -march=native -mtune=native -Wall -Wextra -Ofast -ftree-vectorize -flto -c gmp256k1/Random.cpp -o Random.o
	g++ -march=native -mtune=native -Wall -Wextra -Ofast -ftree-vectorize -flto -c gmp256k1/IntGroup.cpp -o IntGroup.o
	g++ -march=native -mtune=native -Wall -Wextra -Ofast -ftree-vectorize -o keyhunt keyhunt_legacy.cpp base58.o bloom.o oldbloom.o xxhash.o util.o Int.o  Point.o GMP256K1.o  IntMod.o  IntGroup.o Random.o hashing.o sha3.o keccak.o -lm -lpthread -lcrypto -lgmp	
	rm -r *.o
bsgsd:
	g++ $(BASE_CXXFLAGS) -flto -c oldbloom/bloom.cpp -o oldbloom.o
	g++ $(BASE_CXXFLAGS) -flto -c bloom/bloom.cpp -o bloom.o
	gcc $(BASE_CFLAGS) -Wno-unused-parameter -c base58/base58.c -o base58.o
	gcc $(BASE_CFLAGS) -c rmd160/rmd160.c -o rmd160.o
	g++ $(BASE_CXXFLAGS) -c sha3/sha3.c -o sha3.o
	g++ $(BASE_CXXFLAGS) -c sha3/keccak.c -o keccak.o
	gcc $(BASE_CFLAGS) -c xxhash/xxhash.c -o xxhash.o
	g++ $(BASE_CXXFLAGS) -c util.c -o util.o
	g++ $(BASE_CXXFLAGS) -c cpu_features.cpp -o cpu_features.o
	g++ $(BASE_CXXFLAGS) -c secp256k1/Int.cpp -o Int.o
	g++ $(BASE_CXXFLAGS) -c secp256k1/Point.cpp -o Point.o
	g++ $(BASE_CXXFLAGS) -c secp256k1/SECP256K1.cpp -o SECP256K1.o
	g++ $(BASE_CXXFLAGS) -c secp256k1/IntMod.cpp -o IntMod.o
	g++ $(BASE_CXXFLAGS) -flto -c secp256k1/Random.cpp -o Random.o
	g++ $(BASE_CXXFLAGS) -flto -c secp256k1/IntGroup.cpp -o IntGroup.o
	g++ $(BASE_CXXFLAGS) -flto -c hash/ripemd160.cpp -o hash/ripemd160.o
	g++ $(BASE_CXXFLAGS) -flto -c hash/sha256.cpp -o hash/sha256.o
	g++ $(BASE_CXXFLAGS) -flto -c hash/ripemd160_sse.cpp -o hash/ripemd160_sse.o
	g++ $(BASE_CXXFLAGS) -flto -c hash/sha256_sse.cpp -o hash/sha256_sse.o
	g++ $(BASE_CXXFLAGS) -mavx2 -c hash/ripemd160_avx2.cpp -o hash/ripemd160_avx2.o
	g++ $(BASE_CXXFLAGS) -mavx2 -c hash/sha256_avx2.cpp -o hash/sha256_avx2.o
	g++ $(BASE_CXXFLAGS) -o bsgsd bsgsd.cpp base58.o rmd160.o hash/ripemd160.o hash/ripemd160_sse.o hash/ripemd160_avx2.o hash/sha256.o hash/sha256_sse.o hash/sha256_avx2.o bloom.o oldbloom.o xxhash.o util.o cpu_features.o Int.o Point.o SECP256K1.o IntMod.o Random.o IntGroup.o sha3.o keccak.o -lm -lpthread
	rm -r *.o
