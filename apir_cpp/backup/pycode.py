from cryptography.hazmat.primitives.asymmetric import ec
import gmpy2
import secrets
import random
import time
def generate_large_prime(size):
    """
    生成一个指定位数的大素数
    """
    while True:
        p = random.getrandbits(size) | (1 << size-1) | 1
        if gmpy2.is_prime(p):
            return p

def Select_H(G_order, n):
    elements =[]
    while len(elements) < n:
        # 随机选择一个元素
        element = secrets.randbelow(G_order - 1) + 1
        elements.append(element)
    return elements


def FullProcess(n,bitlenght):
    bit_length = bitlenght-1
    p = generate_large_prime(bit_length + 1)

    d = 1

    db = []

    for i in range(n):
        db.append(random.random()%2)
    db[1] = 0

    time0 = time.perf_counter()
    h = Select_H(p, n)
    for i in range(n):
        temp = pow(h[i], db[i], p)  # Line1
        d = gmpy2.mod(gmpy2.mul(temp, d), p)

    time1 = time.perf_counter()

    # QueryProcess
    Queryindex = 1
    r = generate_large_prime(511)
    t = generate_large_prime(511)
    q = []

    for i in range(n):
        if i != Queryindex:
            q.append(pow(h[i], r, p))  # Query - Line2
        else:
            q.append(gmpy2.powmod(h[i], gmpy2.add(r, t), p))
    time2 = time.perf_counter()
    # AnswerProcess
    a = 1
    for i in range(n):
        a = gmpy2.mod(gmpy2.mul(pow(q[i], db[i], p), a), p)

    # ReconProcess
    d = gmpy2.invert(d, p)
    m = gmpy2.mod(gmpy2.mul(pow(d, r, p), a), p)
    print(f"db[{Queryindex}] = {db[Queryindex]}\nm = {m}")
    time3 = time.perf_counter()
    # print("结果为" + str(m))
    print("初始化耗时为:" + str(time1 - time0))
    print("查询构建耗时为:" + str(time2 - time1))
    print("重构操作耗时为:" + str(time3 - time2))


if __name__ == '__main__':
    # for i in range(1,20):
    #     print("\n当前数据数量为:"+str(1024*pow(2,i)))
    #     FullProcess(1024*pow(2,i),512)
    n = 3
    print("\n当前数据数量为:"+str(n))
    FullProcess(n,512)