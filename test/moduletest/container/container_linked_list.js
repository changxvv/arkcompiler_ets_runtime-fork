/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * @tc.name:container
 * @tc.desc:test container
 * @tc.type: FUNC
 * @tc.require: issueI5NO8G
 */
var LinkedList = undefined;
if (globalThis["ArkPrivate"] != undefined) {
    LinkedList = ArkPrivate.Load(ArkPrivate.LinkedList);
    let list = new LinkedList();
    let testArray = []
    let map = new Map();
    for(let i = 0; i<10; i++) {
        list.add(i)
        testArray.push(i)
    }
    map.set("test linkedlist has:",  list.has(8))
    map.set("test linkedlist not has:", list.has(2))
    map.set("test linkedlist getLastIndexOf:", list.getLastIndexOf(1) === 1)
    map.set("test linkedlist getIndexOf:", list.getIndexOf(5) === 5)

    list.removeByIndex(9)

    testArray.splice(9, 1)
    let res = true
    for(let i = 0; i < testArray.length; i++) {
        if (list[i] !== testArray[i]) {
            res = false
        }
    }
    map.set("test linkedlist removeByIndex:", res)

    const removeRes = list.remove(8)
    testArray.splice(8, 1)
    res = true
    for(let i = 0; i < testArray.length; i++) {
        if (list[i] !== testArray[i]) {
            res = false
        }
    }
    map.set("test linkedlist remove:", res)
    map.set("test linkedlist remove1:", removeRes)
    map.set("test linkedlist getFirst:", list.getFirst() === 0)
    map.set("test linkedlist getLast:", list.getLast() === 7)

    list.insert(3, 999)
    testArray.splice(3, 0, 999)
    res = true
    for(let i = 0; i < testArray.length; i++) {
        if (list[i] !== testArray[i]) {
            res = false
        }
    }
    map.set("test linkedlist insert:", res)

    list.set(5, 888)
    testArray[5] = 888
    res = true
    for(let i = 0; i < testArray.length; i++) {
        if (list[i] !== testArray[i]) {
            res = false
        }
    }
    map.set("test linkedlist set:", res)
    map.set("test linkedlist clone:", res)

    list.addFirst(1111)
    map.set("test linkedlist addfirst:", list.getFirst() === 1111)

    const removefirstres = list.removeFirst()
    map.set("test linkedlist removeFirst:", removefirstres === 1111)

    res = true
    let i = 0
    for (const data of list) {
        if (data !== testArray[i]) {
            res = false
        }
        i++;
    }
    map.set("test linkedlist intertor:", res)

    let list1 = new LinkedList();
    let testArray1 = []
    for (let i = 0; i < 10; i++) {
        list1.add(i)
        testArray1.push(i)
    }

    res = true
    list1.forEach((i, d) => {
        if (d !== testArray1[i]) {
            res = false
        }
    })

    map.set("test linkedlist forEach:", res)
    list1.clear()
    map.set("test linkedlist clear:", list1.length === 0)
    map.set("test linkedlist get:", list.get(1232) === undefined)
    map.set("test linkedlist getLastIndexOf:", list.getLastIndexOf('abc') === -1)
    let flag = false;
    try {
        list.removeByIndex(99)
    } catch (error) {
        flag = true;
    }
    map.set("test linkedlist removeByIndex:", flag)

    testArray.splice(5, 1)

    res = true
    const arr = list.convertToArray()
    for (let i = 1; i < arr.length; i++) {
        if (arr[i] !== testArray[i]) {
            res = false
        }
    }
    map.set("test linkedlist convertToArray:", res)

    let list2 = new LinkedList();
    let proxy = new Proxy(list2, {});
    let testArray2 = []
    for(let i = 0; i<10; i++) {
        proxy.add(i)
        testArray2.push(i)
    }
    map.set("test linkedlist has:",  proxy.has(8))
    map.set("test linkedlist not has:", proxy.has(2))
    map.set("test linkedlist getLastIndexOf:", proxy.getLastIndexOf(1) === 1)
    map.set("test linkedlist getIndexOf:", proxy.getIndexOf(5) === 5)

    proxy.removeByIndex(9)

    testArray2.splice(9, 1)
    res = true
    for(let i = 0; i < testArray2.length; i++) {
        if (proxy[i] !== testArray2[i]) {
            res = false
        }
    }
    map.set("test linkedlist removeByIndex:", res)

    const removeRes1 = proxy.remove(8)
    testArray2.splice(8, 1)
    res = true
    for(let i = 0; i < testArray2.length; i++) {
        if (proxy[i] !== testArray2[i]) {
            res = false
        }
    }
    map.set("test linkedlist remove:", res)
    map.set("test linkedlist remove1:", removeRes1)
    map.set("test linkedlist getFirst:", proxy.getFirst() === 0)
    map.set("test linkedlist getLast:", proxy.getLast() === 7)

    proxy.insert(3, 999)
    testArray2.splice(3, 0, 999)
    res = true
    for(let i = 0; i < testArray2.length; i++) {
        if (proxy[i] !== testArray2[i]) {
            res = false
        }
    }
    map.set("test linkedlist insert:", res)

    proxy.set(5, 888)
    testArray2[5] = 888
    res = true
    for(let i = 0; i < testArray2.length; i++) {
        if (proxy[i] !== testArray2[i]) {
            res = false
        }
    }
    map.set("test linkedlist set:", res)
    map.set("test linkedlist clone:", res)

    proxy.addFirst(1111)
    map.set("test linkedlist addfirst:", proxy.getFirst() === 1111)

    const removefirstres1 = proxy.removeFirst()
    map.set("test linkedlist removeFirst:", removefirstres1 === 1111)

    res = true
    let j = 0
    for (const data of proxy) {
      if (data !== testArray2[j]) {
        res = false
      }
      j++;
    }

    let itr = proxy[Symbol.iterator]();
    let tmp = undefined;
    let arr3 = []
    do {
      tmp = itr.next().value;
      arr3.push(tmp);
    } while (tmp != undefined);

    for (let k = 0; k < proxy.length; k++) {
      if (arr3[k] !== testArray2[k]) {
        res = false
      }
    }

    map.set("test linkedlist intertor:", res)

    let list3 = new LinkedList();
    let proxy1 = new Proxy(list3, {});
    let testArray3 = []
    for (let i = 0; i < 10; i++) {
        proxy1.add(i)
        testArray3.push(i)
    }

    res = true
    proxy1.forEach((i, d) => {
        if (d !== testArray3[i]) {
            res = false
        }
    })

    map.set("test linkedlist forEach:", res)
    proxy1.clear()
    map.set("test linkedlist clear:", proxy1.length === 0)
    map.set("test linkedlist get:", proxy1.get(1232) === undefined)
    map.set("test linkedlist getLastIndexOf:", proxy1.getLastIndexOf('abc') === -1)
    flag = false;
    try {
        proxy1.removeByIndex(99)
    } catch (error) {
        flag = true;
    }
    map.set("test linkedlist removeByIndex:", flag)

    testArray3.splice(5, 1)

    res = true
    const arr1 = proxy1.convertToArray()
    for (let i = 1; i < arr1.length; i++) {
        if (arr1[i] !== testArray3[i]) {
            res = false
        }
    }
    map.set("test linkedlist convertToArray:", res)

    flag = undefined;
    function elements(value, key, map) {
        if (!value) {
            if (!flag) {
                flag = [];
            }
            flag.push(key);
        }
    }
    map.forEach(elements);
    if (!flag) {
        print("Test LinkedList success!!!");
    } else {
        print("Test LinkedList fail: " + flag);
    }
}