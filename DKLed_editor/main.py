# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.
#from PyQt5 import QtWidgets, uic
#import sys
#from ReadDKLedLib import Command_Library_Demonstration

def Mul(a01, a02):
    res = []
    a1 = []
    a2 = []
    for i in range(len(a01)):
        a1.append(a01[i])
    for i in range(len(a02)):
        a2.append(a02[i])
    #a1 = Minim(a1)
    #a2 = Minim(a2)
    #a2.append(a2[len(a2) - 1])
    #a1.append(a1[len(a1) - 1])

    Equalize(a1, a2)
    l = len(a1) #=4
    #print("len A1 = "+ str(len(a1))+ ", len A2 = "+str(len(a2)))
    for i in range(l): #0123
        r = 0
        #print("r(" + str(i) + "):")
        for j in range(l): #0123
            k = l-j+i #4321 5432 6543 7654
            if k>=l: #>=4
                k = k-l #0321 1032 2103 3210
            #print("r(" + str(i)+") + a1("+str(k)+") * a2("+str(j)+")")
            r = r + a2[j]*a1[k]
        #r =
        #i = 0: 0*0+1*3+2*2+3*1
        res.append(r)
    return res

def Mul2(a1, a2):
    #not comutative i.e. a(b+c) != ab+ac in case of D(a) === dimention(a)<D(c) & D(b)<D(c)
    res = []
    a1 = Minim2(a1)
    a2 = Minim2(a2)
    #a2.append(a2[len(a2) - 1])
    #a1.append(a1[len(a1) - 1])

    while len(a1)!=len(a2):
        if len(a1)>len(a2):
            a2.append(a2[len(a2)-1])
        else:
            a1.append(a1[len(a1) - 1])
    l = len(a1) #=4
    #print("len A1 = "+ str(len(a1))+ ", len A2 = "+str(len(a2)))
    for i in range(l): #0123
        r = 0
        #print("r(" + str(i) + "):")
        for j in range(l): #0123
            k = l-j+i #4321 5432 6543 7654
            if k>=l: #>=4
                k = k-l #0321 1032 2103 3210
            #print("r(" + str(i)+") + a1("+str(k)+") * a2("+str(j)+")")
            r = r + a2[j]*a1[k]
        #r =
        #i = 0: 0*0+1*3+2*2+3*1
        res.append(r)
    #res.pop(len(res) - 1)
    return res

def Equalize(a1, a2):
    while len(a1)!=len(a2):
        if len(a1)>len(a2):
            a2.append(a2[len(a2)-1])
        else:
            a1.append(a1[len(a1) - 1])

def Minim(a1, b=0):
    l = len(a1) #=4
    if b == 0:
        b = l
    m = a1[0]
    res = []
    for i in range(l):  # 0123
        if m>a1[i] and i<b:
            m = a1[i]
    #print("minimum = " + str(m))
    for i in range(b):
        if i<l:
            res.append(a1[i] - m)
        else:
            res.append(a1[l-1] - m)
    return res

def Minim2(a1):
    l = len(a1) #=4
    m = a1[0]
    res = []
    for i in range(l):  # 0123
        if m>a1[i]:
            m = a1[i]
    #print("minimum = " + str(m))
    for i in range(l):
        res.append(a1[i] - m)
    if len(res)>1:
        while res[len(res)-1] == res[len(res)-2]:
            res.pop(len(res)-1)
    return res

def Sum(a01, a02):
    res = []
    a1 = []
    a2 = []
    for i in range(len(a01)):
        a1.append(a01[i])
    for i in range(len(a02)):
        a2.append(a02[i])
    Equalize(a1,a2)
    l = len(a1) #=4
    for i in range(l):
        res.append(a1[i] + a2[i])
    return res

def Dif(a01, a02):
    res = []
    a1 = []
    a2 = []
    for i in range(len(a01)):
        a1.append(a01[i])
    for i in range(len(a02)):
        a2.append(a02[i])
    Equalize(a1, a2)
    l = len(a1)  # =4
    for i in range(l):
        res.append(a1[i] - a2[i])
    res = Minim(res)
    return res


Arr1 = [0,1,2,2]
Arr2 = [0,3,5,5]
Arr3 = [2,4,6,7,4]

print('Arr1:')
print(Arr1)
print(Minim(Arr1))

print('Arr2:')
print(Arr2)
print(Minim(Arr2))

Arr4 = Mul(Arr1, Arr2)
print("Arr1 * Arr2 = ")
print(Arr4)

Arr4 = Mul(Arr2, Arr1)
print("Arr2 * Arr1 = ")
print(Arr4)

Arr4 = Minim(Arr4)
print("Arr4 minimal form = ")
print(Arr4)

print("A1*(A2+A3)=")
print(Minim(Mul(Arr1, Sum(Arr2, Arr3))))
print(Minim(Mul2(Arr1, Sum(Arr2, Arr3))))
print("A1*A2+A1*A3=")
print(Minim(Sum(Mul(Arr1, Arr2), Mul(Arr1,Arr3))))
print(Minim(Sum(Mul2(Arr1, Arr2), Mul2(Arr1,Arr3))))

print()
print("different dimentions")
Arr5 = [1,4,0,3]
Arr6 = [7,1,0]
Arr7 = []
for i in range(len(Arr5)):
    Arr7.append(Arr5[i])
Arr7.append(Arr7[len(Arr7)-1])

print(str(Arr5) + " x "+ str(Arr6))
print(Minim(Mul(Arr5, Arr6)))
print(Minim(Mul2(Arr5, Arr6)))
print(str(Arr7) + " x "+ str(Arr6))
print(Minim(Mul(Arr7, Arr6)))
print(Minim(Mul2(Arr7, Arr6)))

#print(Minim(Mul([0,1,2,2], [0,1,2,2])))
#print(Minim(Mul([0,1,2,2,2], [0,1,2,2,2])))
print()

print("Shifting dimentions")
print(Minim(Mul([0,1,2,2], [0,3,5,5]),2))
print(Minim(Mul([0,1,2,2], [0,3,5,5]),3))
print(Minim(Mul([0,1,2,2], [0,3,5,5])))
print(Minim(Mul([2,0,1,2], [5,0,3,5])))

print(Minim(Mul2([0,1,2,2], [0,3,5,5])))
print(Minim(Mul2([2,0,1,2], [5,0,3,5])))
print()
print("different extencions of same numbers")
print(Minim(Mul([3,0,2], [0,2,1])))
print(Minim(Mul([3,0,2,2], [0,2,1,1]),3))
print(Minim(Mul([3,0,2,2,2], [0,2,1,1,1]),3))
print(Minim(Mul([1,5,0,0,0], [1,0,0,0,0]),3))
print(Minim(Mul([5,0,1,1,1], [0,1,0,0,0]),3))
print()

arr12 = []
arr12.append(Arr1[0])
arr12.append(Arr1[2])
arr2 = []
arr2.append(Arr1)
arr2.append(arr12)

PP = 0
print(PP)
PP = PP + 1
print (PP)



Arr1.clear()
Arr1 = [5,6]
arr2.append(Arr1)

print(arr2)
print(str(arr2[1]))
print(str(arr2[1][0]))

print(str(int("123")))


text = ' "text text "'
print("["+text+"]")
text = text.strip(" ").replace('"','')
print("["+text+"]")

print(str(" 12Fa   ".replace(" ", "") in "0123456789abcdefABCDEF"))
print("int(one)" + str(int("   12Ad  ",16)))

print(str(8//3))

print()
print("hex rep:__"+hex(14)[2])
print()

class A0:
    A="5"
    B="6"

print(A0.A + "__"+ A0.B)
A0.A = "A0.B"
print(A0.A + "__"+ A0.B)

#Command_Library_Demonstration()
#ui.show()
#app.exec()

#import DKLedMain