# 예제 1번
a,b,c = map(int, input('세 정수를 입력하시오 :').split())
if b<a and b<c:
    a,b=b,a
if c<b and c<a:
    a,c=c,a
if c<b:
    b,c=c,b
print(a,b,c)
