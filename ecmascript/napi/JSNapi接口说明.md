## ArrayBufferRef

ArrayBufferRef ��һ��ͨ�õġ��̶����ȵ�ԭʼ���������ݻ�������������ֱ�Ӷ�ȡ���������Ҫͨ����������� DataView �������������е����ݡ�

### New

Local<ArrayBufferRef> ArrayBufferRef::New(const EcmaVM *vm, int32_t length)��

Local<ArrayBufferRef> ArrayBufferRef::New(const EcmaVM *vm, void *buffer, int32_t length, const Deleter &deleter, void *data)��

����һ��ArrayBuffer����

**������**

| ������  | ����            | ���� | ˵��                        |
| ------- | --------------- | ---- | --------------------------- |
| vm      | const EcmaVM *  | ��   | ���������                |
| length  | int32_t         | ��   | ָ���ĳ��ȡ�                |
| buffer  | void *          | ��   | ָ����������                |
| deleter | const Deleter & | ��   | ɾ��ArrayBufferʱ�����Ĳ��� |
| data    | void *          | ��   | ָ�����ݡ�                  |

**����ֵ��**

| ����                  | ˵��                             |
| --------------------- | -------------------------------- |
| Local<ArrayBufferRef> | �����´�����ArrayBufferRef���� |

**ʾ����**

```C++
Local<ArrayBufferRef> arrayBuffer1 = ArrayBufferRef::New(vm, 10);
uint8_t *buffer = new uint8_t[10]();
int *data = new int;
*data = 10;
Deleter deleter = [](void *buffer, void *data) -> void {
    delete[] reinterpret_cast<uint8_t *>(buffer);
    int *currentData = reinterpret_cast<int *>(data);
    delete currentData;
};
Local<ArrayBufferRef> arrayBuffer2 = ArrayBufferRef::New(vm, buffer, 10, deleter, data);
```

### GetBuffer

void *ArrayBufferRef::GetBuffer()��

��ȡArrayBufferRef��ԭʼ��������

**������**

| ������ | ���� | ���� | ˵�� |
| ------ | ---- | ---- | ---- |
| �޲�   |      |      |      |

**����ֵ��**

| ����   | ˵��                                             |
| ------ | ------------------------------------------------ |
| void * | ����Ϊvoid *��ʹ��ʱ��ǿתΪ����������ʱ�����͡� |

**ʾ����**

```c++
uint8_t *buffer = new uint8_t[10]();
int *data = new int;
*data = 10;
Deleter deleter = [](void *buffer, void *data) -> void {
delete[] reinterpret_cast<uint8_t *>(buffer);
    int *currentData = reinterpret_cast<int *>(data);
    delete currentData;
};
Local<ArrayBufferRef> arrayBuffer = ArrayBufferRef::New(vm, buffer, 10, deleter, data);
uint8_t *getBuffer = reinterpret_cast<uint8_t *>(arrayBuffer->GetBuffer());
```

### ByteLength

int32_t  ArrayBufferRef::ByteLength([[maybe_unused]] const EcmaVM *vm)��

�˺������ش�ArrayBufferRef�������ĳ��ȣ����ֽ�Ϊ��λ����

**������**

| ������ | ����           | ���� | ˵��         |
| ------ | -------------- | ---- | ------------ |
| vm     | const EcmaVM * | ��   | ��������� |

**����ֵ��**

| ����    | ˵��                            |
| ------- | ------------------------------- |
| int32_t | ��int32_t���ͷ���buffer�ĳ��ȡ� |

**ʾ����**

```c++
Local<ArrayBufferRef> arrayBuffer = ArrayBufferRef::New(vm, 10);
int lenth = arrayBuffer->ByteLength(vm);
```

### IsDetach

bool ArrayBufferRef::IsDetach()��

�ж�ArrayBufferRef���仺�����Ƿ��Ѿ����롣

**������**

| ������ | ����           | ���� | ˵��         |
| ------ | -------------- | ---- | ------------ |
| vm     | const EcmaVM * | ��   | ��������� |

**����ֵ��**

| ���� | ˵��                                      |
| ---- | ----------------------------------------- |
| bool | �������Ѿ����뷵��true��δ���뷵��false�� |

**ʾ����**

```C++
Local<ArrayBufferRef> arrayBuffer = ArrayBufferRef::New(vm, 10);
bool b = arrayBuffer->IsDetach();
```

### Detach

void ArrayBufferRef::Detach(const EcmaVM *vm)��

��ArrayBufferRef���仺�������룬����������������Ϊ0��

**������**

| ������ | ���� | ���� | ˵�� |
| ------ | ---- | ---- | ---- |
| �޲�   |      |      |      |

**����ֵ��**

| ���� | ˵��                             |
| ---- | -------------------------------- |
| void | ��ArrayBufferRef���仺�������롣 |

**ʾ����**

```C++
Local<ArrayBufferRef> arrayBuffer = ArrayBufferRef::New(vm, 10);
arrayBuffer->Detach(vm);
```

## BooleanRef

Boolean��һ�������������ͣ����ڱ�ʾ���١�

### New

Local<BooleanRef> BooleanRef::New(const EcmaVM *vm, bool input)��

����һ��BooleanRef����

**������**

| ������ | ����           | ���� | ˵��                         |
| ------ | -------------- | ---- | ---------------------------- |
| vm     | const EcmaVM * | ��   | ������������������     |
| input  | bool           | ��   | ָ��BooleanRef�����boolֵ�� |

**����ֵ��**

| ����              | ˵��                         |
| ----------------- | ---------------------------- |
| Local<BooleanRef> | �����´�����BooleanRef���� |

**ʾ����**

```c++
Local<BooleanRef> boolRef = BooleanRef::New(vm, true);
```

## BufferRef

���������ݴ�һ��λ�ô��䵽��һ��λ��ʱ��ʱ�洢���ݡ�

### New

Local<BufferRef> BufferRef::New(const EcmaVM *vm, int32_t length);

Local<BufferRef> BufferRef::New(const EcmaVM *vm, void *buffer, int32_t length, const Deleter &deleter, void *data)

����һ��BufferRef����

**������**

| ������  | ����            | ���� | ˵��                                               |
| ------- | --------------- | ---- | -------------------------------------------------- |
| vm      | const EcmaVM *  | ��   | ���������                                       |
| length  | int32_t         | ��   | ָ���ĳ��ȡ�                                       |
| buffer  | void *          | ��   | ָ��������                                         |
| deleter | const Deleter & | ��   | һ��ɾ�������������ڲ�����Ҫ������ʱ�ͷ����ڴ档 |
| data    | void *          | ��   | ���ݸ�ɾ�����Ķ������ݡ�                           |

**����ֵ��**

| ����             | ˵��                        |
| ---------------- | --------------------------- |
| Local<BufferRef> | �����´�����BufferRef���� |

**ʾ����**

```c++
Local<BufferRef> bufferRef1 = BufferRef::New(vm, 10);
uint8_t *buffer = new uint8_t[10]();
int *data = new int;
*data = 10;
Deleter deleter = [](void *buffer, void *data) -> void {
    delete[] reinterpret_cast<uint8_t *>(buffer);
    int *currentData = reinterpret_cast<int *>(data);
    delete currentData;
};
Local<BufferRef> bufferRef2 = BufferRef::New(vm, buffer, 10, deleter, data);
```

### ByteLength

int32_t BufferRef::ByteLength(const EcmaVM *vm)��

�˺������ش�ArrayBufferRef�������ĳ��ȣ����ֽ�Ϊ��λ����

**������**

| ������ | ����           | ���� | ˵��         |
| :----: | -------------- | ---- | ------------ |
|   vm   | const EcmaVM * | ��   | ��������� |

**����ֵ��**

| ����    | ˵��                            |
| ------- | ------------------------------- |
| int32_t | ��int32_t���ͷ���buffer�ĳ��ȡ� |

**ʾ����**

```c++
Local<BufferRef> buffer = BufferRef::New(vm, 10);
int32_t lenth = buffer->ByteLength(vm);
```

### GetBuffer

void *BufferRef::GetBuffer()��

��ȡBufferRef��ԭʼ��������

**������**

| ������ | ���� | ���� | ˵�� |
| ------ | ---- | ---- | ---- |
| �޲�   |      |      |      |

**����ֵ��**

| ����   | ˵��                                             |
| ------ | ------------------------------------------------ |
| void * | ����Ϊvoid *��ʹ��ʱ��ǿתΪ����������ʱ�����͡� |

**ʾ����**

```c++
uint8_t *buffer = new uint8_t[10]();
int *data = new int;
*data = 10;
Deleter deleter = [](void *buffer, void *data) -> void {
    delete[] reinterpret_cast<uint8_t *>(buffer);
    int *currentData = reinterpret_cast<int *>(data);
    delete currentData;
};
Local<BufferRef> bufferRef = BufferRef::New(vm, buffer, 10, deleter, data);
uint8_t *getBuffer = reinterpret_cast<uint8_t *>(bufferRef->GetBuffer());
```
### BufferToStringCallback

static ecmascript::JSTaggedValue BufferToStringCallback(ecmascript::EcmaRuntimeCallInfo *ecmaRuntimeCallInfo);

����ToString����ʱ�ᴥ���Ļص�������

**������**

| ������              | ����                   | ���� | ˵��                                   |
| ------------------- | ---------------------- | ---- | -------------------------------------- |
| ecmaRuntimeCallInfo | EcmaRuntimeCallInfo  * | ��   | ����ָ���Ļص��������Լ�����Ҫ�Ĳ����� |

**����ֵ��**

| ����          | ˵��                                                         |
| ------------- | ------------------------------------------------------------ |
| JSTaggedValue | �����ûص������Ľ��ת��ΪJSTaggedValue���ͣ�����Ϊ�˺����ķ���ֵ�� |

**ʾ����**

```C++
Local<BufferRef> bufferRef = BufferRef::New(vm, 10);
Local<StringRef> bufferStr = bufferRef->ToString(vm);
```

## DataViewRef

һ�����ڲ������������ݵ���ͼ�������ṩ��һ�ַ�ʽ�����ʺ��޸� ArrayBuffer �е��ֽڡ�

### New

static Local<DataViewRef> New(const EcmaVM *vm, Local<ArrayBufferRef> arrayBuffer, uint32_t byteOffset,uint32_t byteLength)��

����һ���µ�DataView����

**������**

| ������      | ����                  | ���� | ˵��                       |
| ----------- | --------------------- | ---- | -------------------------- |
| vm          | const EcmaVM *        | ��   | ���������               |
| arrayBuffer | Local<ArrayBufferRef> | ��   | ָ���Ļ�������             |
| byteOffset  | uint32_t              | ��   | �ӵڼ����ֽڿ�ʼд�����ݡ� |
| byteLength  | uint32_t              | ��   | Ҫ�����Ļ������ĳ��ȡ�     |

**����ֵ��**

| ����               | ˵��                   |
| ------------------ | ---------------------- |
| Local<DataViewRef> | һ���µ�DataView���� |

**ʾ����**

```c++
const int32_t length = 15;
Local<ArrayBufferRef> arrayBuffer = ArrayBufferRef::New(vm, length);
Local<DataViewRef> dataObj = DataViewRef::New(vm, arrayBuffer, 5, 7);
```

### ByteOffset

uint32_t DataViewRef::ByteOffset()��

��ȡDataViewRef��������ƫ������

**������**

| ������ | ���� | ���� | ˵�� |
| ------ | ---- | ---- | ---- |
| �޲�   |      |      |      |

**����ֵ��**

| ����     | ˵��                               |
| -------- | ---------------------------------- |
| uint32_t | �û��������Ǹ��ֽڿ�ʼд����ȡ�� |

**ʾ����**

```C++
const int32_t length = 15;
Local<ArrayBufferRef> arrayBuffer = ArrayBufferRef::New(vm, length);
Local<DataViewRef> dataView = DataViewRef::New(vm, arrayBuffer, 5, 7);
uint32_t offSet = dataView->ByteOffset();
```

### ByteLength

uint32_t DataViewRef::ByteLength()��

��ȡDataViewRef�������ɲ����ĳ��ȡ�

**������**

| ������ | ���� | ���� | ˵�� |
| ------ | ---- | ---- | ---- |
| �޲�   |      |      |      |

**����ֵ��**

| ����     | ˵��                            |
| -------- | ------------------------------- |
| uint32_t | DataViewRef�������ɲ����ĳ��ȡ� |

**ʾ����**

```C++
const int32_t length = 15;
Local<ArrayBufferRef> arrayBuffer = ArrayBufferRef::New(vm, length);
Local<DataViewRef> dataView = DataViewRef::New(vm, arrayBuffer, 5, 7);
uint32_t offSet = dataView->ByteLength();
```

### GetArrayBuffer

Local<ArrayBufferRef> DataViewRef::GetArrayBuffer(const EcmaVM *vm)��

��ȡDataViewRef����Ļ�������

**������**

| ������ | ����           | ���� | ˵��         |
| :----: | -------------- | ---- | ------------ |
|   vm   | const EcmaVM * | ��   | ��������� |

**����ֵ��**

| ����                  | ˵��                                                         |
| --------------------- | ------------------------------------------------------------ |
| Local<ArrayBufferRef> | ��ȡDataViewRef�Ļ�����������ת��ΪLocal<ArrayBufferRef>���ͣ�����Ϊ�����ķ���ֵ�� |

**ʾ����**

```c++
const int32_t length = 15;
Local<ArrayBufferRef> arrayBuffer = ArrayBufferRef::New(vm, length);
Local<DataViewRef> dataView = DataViewRef::New(vm, arrayBuffer, 5, 7);
Local<ArrayBufferRef> getBuffer = dataView->GetArrayBuffer(vm);
```

## DateRef

Date�������ڱ�ʾ���ں�ʱ�䡣���ṩ����෽�����������������ں�ʱ�䡣

### GetTime

double DateRef::GetTime()��

�÷��������Լ�Ԫ���������ڵĺ��������ü�Ԫ����Ϊ 1970 �� 1 �� 1 �գ�UTC ��ʼʱ����ҹ��

**������**

| ������ | ���� | ���� | ˵�� |
| ------ | ---- | ---- | ---- |
| �޲�   |      |      |      |

**����ֵ��**

| ����   | ˵��                                             |
| ------ | ------------------------------------------------ |
| double | һ��double���֣���ʾ�����ڵ�ʱ����Ժ���Ϊ��λ�� |

**ʾ����**

```c++
double time = 1690854800000; // 2023-07-04T00:00:00.000Z
Local<DateRef> object = DateRef::New(vm, time);
double getTime = object->GetTime();
```

## JSNApi

JSNApi�ṩ��һЩͨ�õĽӿ����ڲ�ѯ���ȡһЩ�������ԡ�

### IsBundle

bool JSNApi::IsBundle(EcmaVM *vm)��

�жϽ��ļ�����ģ����ʱ���ǲ���JSBundleģʽ��

**������**

| ������ | ����           | ���� | ˵��         |
| :----: | -------------- | ---- | ------------ |
|   vm   | const EcmaVM * | ��   | ��������� |

**����ֵ��**

| ���� | ˵��                                          |
| ---- | --------------------------------------------- |
| bool | ��ΪJSBundleģʽʱʱ����true�����򷵻�false�� |

**ʾ����**

```c++
bool b = JSNApi::IsBundle(vm);
```

### addWorker

void JSNApi::addWorker(EcmaVM *hostVm, EcmaVM *workerVm)

����һ���µ��������ӵ�ָ��������Ĺ����б��С�

**������**

|  ������  | ����           | ���� | ˵��                 |
| :------: | -------------- | ---- | -------------------- |
|  hostVm  | const EcmaVM * | ��   | ָ�����������     |
| workerVm | const EcmaVM * | ��   | �����µĹ���������� |

**����ֵ��**

| ���� | ˵��       |
| ---- | ---------- |
| void | �޷���ֵ�� |

**ʾ����**

```c++
JSRuntimeOptions option;
EcmaVM *workerVm = JSNApi::CreateEcmaVM(option);
JSNApi::addWorker(hostVm, workerVm);
```

### SerializeValue

void *JSNApi::SerializeValue(const EcmaVM *vm, Local<JSValueRef> value, Local<JSValueRef> transfer)

��value���л�Ϊtransfer���͡�

**������**

|  ������  | ����              | ���� | ˵��               |
| :------: | ----------------- | ---- | ------------------ |
|    vm    | const EcmaVM *    | ��   | ָ�����������   |
|  value   | Local<JSValueRef> | ��   | ��Ҫ���л������ݡ� |
| transfer | Local<JSValueRef> | ��   | ���л������͡�     |

**����ֵ��**

| ����   | ˵��                                                         |
| ------ | ------------------------------------------------------------ |
| void * | ת��ΪSerializationData *�ɵ���GetData��GetSize��ȡ���л��������Լ���С�� |

**ʾ����**

```c++
Local<JSValueRef> value = StringRef::NewFromUtf8(vm, "abcdefbb");
Local<JSValueRef> transfer = StringRef::NewFromUtf8(vm, "abcdefbb");
SerializationData *ptr = JSNApi::SerializeValue(vm, value, transfer);
```

### DisposeGlobalHandleAddr

static void DisposeGlobalHandleAddr(const EcmaVM *vm, uintptr_t addr);

�ͷ�ȫ�־��

**������**

| ������ | ����           | ���� | ˵��             |
| :----: | -------------- | ---- | ---------------- |
|   vm   | const EcmaVM * | ��   | ָ����������� |
|  addr  | uintptr_t      | ��   | ȫ�־���ĵ�ַ�� |

**����ֵ��**

| ���� | ˵��       |
| ---- | ---------- |
| void | �޷���ֵ�� |

**ʾ����**

```C++
Local<JSValueRef> value = StringRef::NewFromUtf8(vm, "abc");
uintptr_t address = JSNApi::GetGlobalHandleAddr(vm, reinterpret_cast<uintptr_t>(*value));
JSNApi::DisposeGlobalHandleAddr(vm, address);
```

### CheckSecureMem

static bool CheckSecureMem(uintptr_t mem);

���������ڴ��ַ�Ƿ�ȫ��

**������**

| ������ | ����      | ���� | ˵��                 |
| :----: | --------- | ---- | -------------------- |
|  mem   | uintptr_t | ��   | ��Ҫ�����ڴ��ַ�� |

**����ֵ��**

| ���� | ˵��                                |
| ---- | ----------------------------------- |
| bool | �ڴ��ַ��ȫ����true���򷵻�false�� |

**ʾ����**

```c++
Local<JSValueRef> value = StringRef::NewFromUtf8(vm, "abc");
uintptr_t address = JSNApi::GetGlobalHandleAddr(vm, reinterpret_cast<uintptr_t>(*value));
bool b = CheckSecureMem(address);
```

### GetGlobalObject

Local<ObjectRef> JSNApi::GetGlobalObject(const EcmaVM *vm)

�����ܷ�ɹ���ȡJavaScript�������envȫ�ֶ����Լ���ȫ�ֶ����Ƿ�Ϊ�ǿն���

**������**

| ������ | ����           | ���� | ˵��             |
| :----: | -------------- | ---- | ---------------- |
|   vm   | const EcmaVM * | ��   | ָ����������� |

**����ֵ��**

| ����             | ˵��                                                      |
| ---------------- | --------------------------------------------------------- |
| Local<ObjectRef> | �ɵ���ObjectRef�Լ�����JSValueRef�ĺ������ж����Ƿ���Ч�� |

**ʾ����**

```C++
Local<ObjectRef> globalObject = JSNApi::GetGlobalObject(vm);
bool b = globalObject.IsEmpty();
```

### GetUncaughtException

Local<ObjectRef> JSNApi::GetUncaughtException(const EcmaVM *vm)��

��ȡ�����δ������쳣

**������**

| ������ | ����           | ���� | ˵��             |
| :----: | -------------- | ---- | ---------------- |
|   vm   | const EcmaVM * | ��   | ָ����������� |

**����ֵ��**

| ����             | ˵��                                                      |
| ---------------- | --------------------------------------------------------- |
| Local<ObjectRef> | �ɵ���ObjectRef�Լ�����JSValueRef�ĺ������ж����Ƿ���Ч�� |

**ʾ����**

```c++
Local<ObjectRef> excep = JSNApi::GetUncaughtException(vm);
if (!excep.IsNull()) {
    // Error Message ...
}
```

### GetAndClearUncaughtException

Local<ObjectRef> JSNApi::GetAndClearUncaughtException(const EcmaVM *vm)��

���ڻ�ȡ�����δ�����������쳣��

**������**

| ������ | ����           | ���� | ˵��             |
| :----: | -------------- | ---- | ---------------- |
|   vm   | const EcmaVM * | ��   | ָ����������� |

**����ֵ��**

| ����             | ˵��                                                      |
| ---------------- | --------------------------------------------------------- |
| Local<ObjectRef> | �ɵ���ObjectRef�Լ�����JSValueRef�ĺ������ж����Ƿ���Ч�� |

**ʾ����**

```C++
Local<ObjectRef> excep = JSNApi::GetUncaughtException(vm);
if (!excep.IsNull()) {
    // Error Message ...
    JSNApi::GetAndClearUncaughtException(vm);
}
```

### HasPendingException

bool JSNApi::HasPendingException(const EcmaVM *vm)��

���ڼ��������Ƿ���δ������쳣��

**������**

| ������ | ����           | ���� | ˵��             |
| :----: | -------------- | ---- | ---------------- |
|   vm   | const EcmaVM * | ��   | ָ����������� |

**����ֵ��**

| ���� | ˵��                                          |
| ---- | --------------------------------------------- |
| bool | �������������쳣��������true���򷵻�false�� |

**ʾ����**

```c++
if (JSNApi::HasPendingException(vm)) {
    JSNApi::PrintExceptionInfo(const EcmaVM *vm);
}
```

### PrintExceptionInfo

void JSNApi::PrintExceptionInfo(const EcmaVM *vm)��

���ڴ�ӡδ������쳣�����������������쳣��

**������**

| ������ | ����           | ���� | ˵��             |
| :----: | -------------- | ---- | ---------------- |
|   vm   | const EcmaVM * | ��   | ָ����������� |

**����ֵ��**

| ���� | ˵��       |
| ---- | ---------- |
| void | �޷���ֵ�� |

**ʾ����**

```C++
if (JSNApi::HasPendingException(vm)) {
    JSNApi::PrintExceptionInfo(vm);
}
```

### SetWeakCallback

uintptr_t JSNApi::SetWeakCallback(const EcmaVM *vm, uintptr_t localAddress, void *ref, WeakRefClearCallBack freeGlobalCallBack, WeakRefClearCallBack nativeFinalizeCallback)��

�˺�����������һ�����ص����������ص�������һ���������͵Ļص��������������ڲ���Ҫʱ�Զ��ͷ��ڴ棬�Ա����ڴ�й©���⡣

**������**

|         ������         | ����                 | ���� | ˵��                                                       |
| :--------------------: | -------------------- | ---- | ---------------------------------------------------------- |
|           vm           | const EcmaVM *       | ��   | ָ�����������                                           |
|      localAddress      | uintptr_t            | ��   | ���ص�ַ��ָ���õ����������ڴ��ַ                         |
|          ref           | void *               | ��   | ��Ҫ���ö�����ڴ��ַ��                                   |
|   freeGlobalCallBack   | WeakRefClearCallBack | ��   | ����������ص��������������ñ����ʱ���ûص������������á� |
| nativeFinalizeCallback | WeakRefClearCallBack | ��   | C++ԭ�����������������                                    |

**����ֵ��**

| ����      | ˵��                       |
| --------- | -------------------------- |
| uintptr_t | ���������ڴ�ռ��еĵ�ַ�� |

**ʾ����**

```C++
template <typename T> void FreeGlobalCallBack(void *ptr)
{
    T *i = static_cast<T *>(ptr);
}
template <typename T> void NativeFinalizeCallback(void *ptr)
{
    T *i = static_cast<T *>(ptr);
    delete i;
}
Global<JSValueRef> global(vm, BooleanRef::New(vm, true));
void *ref = new char('a');
WeakRefClearCallBack freeGlobalCallBack = FreeGlobalCallBack<char>;
WeakRefClearCallBack nativeFinalizeCallback = NativeFinalizeCallback<char>;
global.SetWeakCallback(ref, freeGlobalCallBack, nativeFinalizeCallback);
```

### ThrowException

void JSNApi::ThrowException(const EcmaVM *vm, Local<JSValueRef> error)��

VM������׳�ָ���쳣��

**������**

| ������ | ����              | ���� | ˵��             |
| :----: | ----------------- | ---- | ---------------- |
|   vm   | const EcmaVM *    | ��   | ָ����������� |
| error  | Local<JSValueRef> | ��   | ָ��error��Ϣ��  |

**����ֵ��**

| ���� | ˵��       |
| ---- | ---------- |
| void | �޷���ֵ�� |

**ʾ����**

```C++
Local<JSValueRef> error = Exception::Error(vm, StringRef::NewFromUtf8(vm, "Error Message"));
JSNApi::ThrowException(vm, error);
```

## JSValueRef

JSValueRef��һ�����ڱ�ʾJSֵ���ࡣ���ṩ��һЩ��ʽ�������ͷ���JS�еĸ����������ͣ����ַ��������֡�����ֵ����������ȡ�ͨ��ʹ��JSValueRef�������Ի�ȡ������JSֵ�����Ժͷ�����ִ�к������ã�ת���������͵ȡ�

### Undefined

Local<PrimitiveRef> JSValueRef::Undefined(const EcmaVM *vm)��

���ڻ�ȡһ����ʾδ����ֵ��`Value`�����������ͨ���ڴ���JavaScript��C++֮�������ת��ʱʹ�á�

**������**

| ������ | ����           | ���� | ˵��             |
| :----: | -------------- | ---- | ---------------- |
|   vm   | const EcmaVM * | ��   | ָ����������� |

**����ֵ��**

| ����                | ˵��             |
| ------------------- | ---------------- |
| Local<PrimitiveRef> | ����Ϊԭ������ |

**ʾ����**

```C++
Local<PrimitiveRef> primitive =JSValueRef::Undefined(vm);
```

### Null

Local<PrimitiveRef> JSValueRef::Null(const EcmaVM *vm)��

���ڻ�ȡһ����ʾΪNull��`Value`�����������ͨ���ڴ���JavaScript��C++֮�������ת��ʱʹ�á�

**������**

| ������ | ����           | ���� | ˵��             |
| :----: | -------------- | ---- | ---------------- |
|   vm   | const EcmaVM * | ��   | ָ����������� |

**����ֵ��**

| ����                | ˵��             |
| ------------------- | ---------------- |
| Local<PrimitiveRef> | ����Ϊԭ������ |

**ʾ����**

```C++
Local<PrimitiveRef> primitive = JSValueRef::Null(vm);
```

### IsGeneratorObject

bool JSValueRef::IsGeneratorObject()��

�ж��Ƿ�Ϊ����������

**������**

| ������ | ���� | ���� | ˵�� |
| ------ | ---- | ---- | ---- |
| �޲�   |      |      |      |

**����ֵ��**

| ���� | ˵��                                  |
| ---- | ------------------------------------- |
| bool | �����������󷵻�true�����򷵻�false�� |

**ʾ����**

```C++
ObjectFactory *factory = vm->GetFactory();
auto env = vm->GetGlobalEnv();
JSHandle<JSTaggedValue> genFunc = env->GetGeneratorFunctionFunction();
JSHandle<JSGeneratorObject> genObjHandleVal = factory->NewJSGeneratorObject(genFunc);
JSHandle<JSHClass> hclass = JSHandle<JSHClass>::Cast(env->GetGeneratorFunctionClass());
JSHandle<JSFunction> generatorFunc = JSHandle<JSFunction>::Cast(factory->NewJSObject(hclass));
JSFunction::InitializeJSFunction(vm->GetJSThread(), generatorFunc, FunctionKind::GENERATOR_FUNCTION);
JSHandle<GeneratorContext> generatorContext = factory->NewGeneratorContext();
generatorContext->SetMethod(vm->GetJSThread(), generatorFunc.GetTaggedValue());
JSHandle<JSTaggedValue> generatorContextVal = JSHandle<JSTaggedValue>::Cast(generatorContext);
genObjHandleVal->SetGeneratorContext(vm->GetJSThread(), generatorContextVal.GetTaggedValue());
JSHandle<JSTaggedValue> genObjTagHandleVal = JSHandle<JSTaggedValue>::Cast(genObjHandleVal);
Local<GeneratorObjectRef> genObjectRef = JSNApiHelper::ToLocal<GeneratorObjectRef>(genObjTagHandleVal);
bool b = genObjectRef->IsGeneratorObject();
```

## FunctionRef

�ṩ������������װΪһ�������Լ��Է�װ�����ĵ��á�

### New

Local<FunctionRef> FunctionRef::New(EcmaVM *vm, FunctionCallback nativeFunc, Deleter deleter, void *data, bool callNapi, size_t nativeBindingsize)��

����һ���µĺ�������

**������**

|      ������       | ����             | ���� | ˵��                                                         |
| :---------------: | ---------------- | ---- | ------------------------------------------------------------ |
|        vm         | const EcmaVM *   | ��   | ָ�����������                                             |
|    nativeFunc     | FunctionCallback | ��   | һ���ص���������JS����������غ���ʱ������������ص�����     |
|      deleter      | Deleter          | ��   | һ��ɾ���������������ڲ�����Ҫ`FunctionRef`����ʱ�ͷ�����Դ�� |
|       data        | void *           | ��   | һ����ѡ��ָ�룬���Դ��ݸ��ص�������ɾ����������             |
|     callNapi      | bool             | ��   | һ������ֵ����ʾ�Ƿ��ڴ���`FunctionRef`����ʱ�������ûص����������Ϊ`true`�����ڴ�������ʱ�������ûص����������Ϊ`false`������Ҫ�ֶ����ûص������� |
| nativeBindingsize | size_t           | ��   | ��ʾnativeFunc�����Ĵ�С��0��ʾδ֪��С��                    |

**����ֵ��**

| ����               | ˵��                     |
| ------------------ | ------------------------ |
| Local<FunctionRef> | ����Ϊһ���µĺ������� |

**ʾ����**

```C++
Local<JSValueRef> FunCallback(JsiRuntimeCallInfo *info)
{
    EscapeLocalScope scope(info->GetVM());
    return scope.Escape(ArrayRef::New(info->GetVM(), info->GetArgsNumber()));
}
Local<FunctionRef> callback = FunctionRef::New(vm, FunCallback);
```

### NewClassFunction

Local<FunctionRef> FunctionRef::NewClassFunction(EcmaVM *vm, FunctionCallback nativeFunc, Deleter deleter, void *data, bool callNapi, size_t nativeBindingsize)��

����һ���µ��ຯ������

**������**

|      ������       | ����             | ���� | ˵��                                                         |
| :---------------: | ---------------- | ---- | ------------------------------------------------------------ |
|        vm         | const EcmaVM *   | ��   | ָ�����������                                             |
|    nativeFunc     | FunctionCallback | ��   | һ���ص���������JS����������غ���ʱ������������ص�����     |
|      deleter      | Deleter          | ��   | һ��ɾ���������������ڲ�����Ҫ`FunctionRef`����ʱ�ͷ�����Դ�� |
|       data        | void *           | ��   | һ����ѡ��ָ�룬���Դ��ݸ��ص�������ɾ����������             |
|     callNapi      | bool             | ��   | һ������ֵ����ʾ�Ƿ��ڴ���`FunctionRef`����ʱ�������ûص����������Ϊ`true`�����ڴ�������ʱ�������ûص����������Ϊ`false`������Ҫ�ֶ����ûص������� |
| nativeBindingsize | size_t           | ��   | ��ʾnativeFunc�����Ĵ�С��0��ʾδ֪��С��                    |

**����ֵ��**

| ����               | ˵��                     |
| ------------------ | ------------------------ |
| Local<FunctionRef> | ����Ϊһ���µĺ������� |

**ʾ����**

```C++
Local<JSValueRef> FunCallback(JsiRuntimeCallInfo *info)
{
    EscapeLocalScope scope(info->GetVM());
    return scope.Escape(ArrayRef::New(info->GetVM(), info->GetArgsNumber()));
}
Deleter deleter = nullptr;
void *cb = reinterpret_cast<void *>(BuiltinsFunction::FunctionPrototypeInvokeSelf);
bool callNative = true;
size_t nativeBindingSize = 15;
Local<FunctionRef> obj(FunctionRef::NewClassFunction(vm, FunCallback, deleter, cb, callNative, nativeBindingSize));
```

### Call

Local<JSValueRef> FunctionRef::Call(const EcmaVM *vm, Local<JSValueRef> thisObj, const Local<JSValueRef> argv[], int32_t length)��

����ָ���������FunctionRef�������õĻص�������

**������**

| ������  | ����                    | ���� | ˵��                            |
| :-----: | ----------------------- | ---- | ------------------------------- |
|   vm    | const EcmaVM *          | ��   | ָ�����������                |
| thisObj | FunctionCallback        | ��   | ָ�����ûص������Ķ���        |
| argv[]  | const Local<JSValueRef> | ��   | Local<JSValueRef>�������顣     |
| length  | int32_t                 | ��   | Local<JSValueRef>�������鳤�ȡ� |

**����ֵ��**

| ����              | ˵��                     |
| ----------------- | ------------------------ |
| Local<JSValueRef> | ���ڷ��غ���ִ�еĽ���� |

**ʾ����**

```C++
Local<JSValueRef> FunCallback(JsiRuntimeCallInfo *info)
{
    EscapeLocalScope scope(info->GetVM());
    return scope.Escape(ArrayRef::New(info->GetVM(), info->GetArgsNumber()));
}
Local<IntegerRef> intValue = IntegerRef::New(vm, 0);
std::vector<Local<JSValueRef>> argumentsInt;
argumentsInt.emplace_back(intValue);
Local<FunctionRef> callback = FunctionRef::New(vm, FunCallback);
callback->Call(vm, JSValueRef::Undefined(vm), argumentsInt.data(), argumentsInt.size());
```

### GetSourceCode

Local<StringRef> GetSourceCode(const EcmaVM *vm, int lineNumber)��

��ȡ���ô˺�����CPP�ļ��ڣ�ָ���кŵ�Դ���롣

**������**

|   ������   | ����           | ���� | ˵��             |
| :--------: | -------------- | ---- | ---------------- |
|     vm     | const EcmaVM * | ��   | ָ����������� |
| lineNumber | int            | ��   | ָ���кš�       |

**����ֵ��**

| ����             | ˵��                  |
| ---------------- | --------------------- |
| Local<StringRef> | ����ΪStringRef���� |

**ʾ����**

```C++
Local<JSValueRef> FunCallback(JsiRuntimeCallInfo *info)
{
    EscapeLocalScope scope(info->GetVM());
    return scope.Escape(ArrayRef::New(info->GetVM(), info->GetArgsNumber()));
}
Local<FunctionRef> callback = FunctionRef::New(vm, FunCallback);
Local<StringRef> src = callback->GetSourceCode(vm, 10);
```

### Constructor

Local<JSValueRef> FunctionRef::Constructor(const EcmaVM *vm, const Local<JSValueRef> argv[], int32_t length)��

����һ����������Ĺ��졣

**������**

| ������ | ����                    | ���� | ˵��                 |
| :----: | ----------------------- | ---- | -------------------- |
|   vm   | const EcmaVM *          | ��   | ָ�����������     |
|  argv  | const Local<JSValueRef> | ��   | �������顣           |
| length | int32_t                 | ��   | argv�����������С�� |

**����ֵ��**

| ����              | ˵��                                                         |
| ----------------- | ------------------------------------------------------------ |
| Local<JSValueRef> | ����һ��FunctionRef��������תΪLocal<JSValueRef>���ͣ���Ϊ��������ֵ�� |

**ʾ����**

```C++
Local<FunctionRef> cls = FunctionRef::NewClassFunction(vm, FunCallback, nullptr, nullptr);
Local<JSValueRef> argv[1];          
int num = 3;                      
argv[0] = NumberRef::New(vm, num);
Local<JSValueRef>functionObj = cls->Constructor(vm, argv, 1); 
```

### GetFunctionPrototype

Local<JSValueRef> FunctionRef::GetFunctionPrototype(const EcmaVM *vm)��

��ȡprototype���������������к�����ԭ�ͷ�������Щ�������Ա����еĺ���ʵ���������д��

**������**

| ������ | ����           | ���� | ˵��             |
| :----: | -------------- | ---- | ---------------- |
|   vm   | const EcmaVM * | ��   | ָ����������� |

**����ֵ��**

| ����              | ˵��                                                         |
| ----------------- | ------------------------------------------------------------ |
| Local<JSValueRef> | ��prototype����תΪLocal<JSValueRef>���ͣ�����Ϊ�˺����ķ���ֵ�� |

**ʾ����**

```C++
Local<JSValueRef> FunCallback(JsiRuntimeCallInfo *info)
{
    EscapeLocalScope scope(info->GetVM());
    return scope.Escape(ArrayRef::New(info->GetVM(), info->GetArgsNumber()));
}
Local<FunctionRef> res = FunctionRef::New(vm, FunCallback);
Local<JSValueRef> prototype = res->GetFunctionPrototype(vm);
```

## TypedArrayRef

һ�����ڴ�����������ݵ����ö�������������ͨ���飬��ֻ�ܴ洢�Ͳ����ض����͵����ݡ�

### ByteLength

uint32_t TypedArrayRef::ByteLength([[maybe_unused]] const EcmaVM *vm)��

�˺������ش�ArrayBufferRef�������ĳ��ȣ����ֽ�Ϊ��λ����

**������**

| ������ | ����           | ���� | ˵��         |
| :----: | -------------- | ---- | ------------ |
|   vm   | const EcmaVM * | ��   | ��������� |

**����ֵ��**

| ����     | ˵��                              |
| -------- | --------------------------------- |
| uint32_t | ��uint32_t ���ͷ���buffer�ĳ��ȡ� |

**ʾ����**

```c++
Local<ArrayBufferRef> arrayBuffer = ArrayBufferRef::New(vm, 10);
Local<DataViewRef> dataView = DataViewRef::New(vm, arrayBuffer, 5, 7);
uint32_t len = dataView->ByteLength();
```

## Exception

�ṩ��һЩ��̬���������ڸ��ݲ�ͬ�Ĵ������ʹ���һ����Ӧ��JS�쳣���󣬲�����һ��ָ��ö�������á�

### AggregateError

static Local<JSValueRef> AggregateError(const EcmaVM *vm, Local<StringRef> message)��

����Ҫ����������װ��һ��������ʱ��AggregateError�����ʾһ������

**������**

| ������  | ����             | ���� | ˵��         |
| :-----: | ---------------- | ---- | ------------ |
|   vm    | const EcmaVM *   | ��   | ��������� |
| message | Local<StringRef> | ��   | ������Ϣ��   |

**����ֵ��**

| ����              | ˵��                                                         |
| ----------------- | ------------------------------------------------------------ |
| Local<JSValueRef> | ����������װΪAggregateError���󣬲�����תΪLocal<JSValueRef>���ͣ���Ϊ�����ķ���ֵ�� |

**ʾ����**

```C++
Local<JSValueRef> error = Exception::AggregateError(vm, StringRef::NewFromUtf8(vm, "test aggregate error"));
```

### EvalError

static Local<JSValueRef> EvalError(const EcmaVM *vm, Local<StringRef> message)��

���ڱ�ʾ��ִ�� `eval()` ����ʱ�����Ĵ��󡣵� `eval()` �����޷�������ִ�д�����ַ�������ʱ�����׳�һ�� `EvalError` �쳣��

**������**

| ������  | ����             | ���� | ˵��         |
| :-----: | ---------------- | ---- | ------------ |
|   vm    | const EcmaVM *   | ��   | ��������� |
| message | Local<StringRef> | ��   | ������Ϣ��   |

**����ֵ��**

| ����              | ˵�� |
| ----------------- | ---- |
| Local<JSValueRef> |      |

**ʾ����**

```C++
Local<JSValueRef> error = Exception::EvalError(vm, StringRef::NewFromUtf8(vm, "test eval error"));
```

## MapIteratorRef

���ڱ�ʾ�Ͳ���JS Map����ĵ��������õ��࣬���̳���ObjectRef�࣬���ṩ��һЩ����JS Map������������

### GetKind

Local<JSValueRef> GetKind(const EcmaVM *vm)��

��ȡMapIterator����Ԫ�ص����ͣ��ֱ�Ϊkey��value��keyAndValue��

**������**

| ������ | ����           | ���� | ˵��         |
| :----: | -------------- | ---- | ------------ |
|   vm   | const EcmaVM * | ��   | ��������� |

**����ֵ��**

| ����              | ˵��                                                         |
| ----------------- | ------------------------------------------------------------ |
| Local<JSValueRef> | ��ȡ�����������Ͳ�����תΪLocal<JSValueRef>����Ϊ�����ķ���ֵ�� |

**ʾ����**

```c++
JSHandle<GlobalEnv> env = vm->GetGlobalEnv();
ObjectFactory *factory = vm->GetFactory();
JSHandle<JSTaggedValue> builtinsMapFunc = env->GetBuiltinsMapFunction();
JSHandle<JSMap> jsMap(factory->NewJSObjectByConstructor(JSHandle<JSFunction>(builtinsMapFunc), builtinsMapFunc));
JSHandle<JSTaggedValue> linkedHashMap(LinkedHashMap::Create(vm->GetJSThread()));
jsMap->SetLinkedMap(vm->GetJSThread(), linkedHashMap);
JSHandle<JSTaggedValue> mapValue(jsMap);
JSHandle<JSTaggedValue> mapIteratorVal = JSMapIterator::CreateMapIterator(vm->GetJSThread(), mapValue, IterationKind::KEY);
JSHandle<JSMapIterator> mapIterator = JSHandle<JSMapIterator>::Cast(mapIteratorVal);
mapIterator->SetIterationKind(IterationKind::VALUE);
mapIterator->SetIterationKind(IterationKind::KEY_AND_VALUE);
Local<MapIteratorRef> object = JSNApiHelper::ToLocal<MapIteratorRef>(mapIteratorVal);
Local<JSValueRef> type = object->GetKind(vm);
```

## PrimitiveRef

����Ϊԭʼ���󣬰���Undefined��Null��Boolean��Number��String��Symbol��BigInt ��ЩPrimitive���͵�ֵ�ǲ��ɱ�ģ���һ�������Ͳ����޸ġ�

### GetValue

Local<JSValueRef> GetValue(const EcmaVM *vm)��

��ȡԭʼ�����ֵ��

**������**

| ������ | ����           | ���� | ˵��         |
| :----: | -------------- | ---- | ------------ |
|   vm   | const EcmaVM * | ��   | ��������� |

**����ֵ��**

| ����              | ˵��                                                         |
| ----------------- | ------------------------------------------------------------ |
| Local<JSValueRef> | ��ȡֵ����ת��Ϊ Local<JSValueRef>���Ͷ��󣬲���Ϊ�����ķ���ֵ�� |

**ʾ����**

```C++
Local<IntegerRef> intValue = IntegerRef::New(vm, 10);
Local<JSValueRef> jsValue = intValue->GetValue(vm);
```

## IntegerRef

���ڱ�ʾһ����������ͨ�����ڴ����������㣬IntegerRef���Դ洢����������������Դ洢16��������

### New

static Local<IntegerRef> New(const EcmaVM *vm, int input)��

����һ���µ�IntegerRef����

**������**

| ������ | ����           | ���� | ˵��                 |
| :----: | -------------- | ---- | -------------------- |
|   vm   | const EcmaVM * | ��   | ���������         |
| input  | int            | ��   | IntegerRef�����ֵ�� |

**����ֵ��**

| ����              | ˵��                         |
| ----------------- | ---------------------------- |
| Local<IntegerRef> | ����һ���µ�IntegerRef���� |

**ʾ����**

```C++
Local<IntegerRef> intValue = IntegerRef::New(vm, 0);
```

### NewFromUnsigned

static Local<IntegerRef> NewFromUnsigned(const EcmaVM *vm, unsigned int input)��

�����޷��ŵ�IntegerRef����

**������**

| ������ | ����           | ���� | ˵��                 |
| :----: | -------------- | ---- | -------------------- |
|   vm   | const EcmaVM * | ��   | ���������         |
| input  | int            | ��   | IntegerRef�����ֵ�� |

**����ֵ��**

| ����              | ˵��                         |
| ----------------- | ---------------------------- |
| Local<IntegerRef> | ����һ���µ�IntegerRef���� |

**ʾ����**

```C++
Local<IntegerRef> intValue = IntegerRef::NewFromUnsigned(vm, 0);
```

## PromiseRef

���ڴ����첽����������ʾһ����δ��ɵ�Ԥ����δ������ɵĲ��������ҷ���һ��ֵ��Promise������״̬��pending�������У���fulfilled���ѳɹ�����rejected����ʧ�ܣ���

### Catch

Local<PromiseRef> Catch(const EcmaVM *vm, Local<FunctionRef> handler)��

���ڲ����첽�����еĴ��󣬵�һ��Promise��rejectedʱ������ʹ��catch�������������

**������**

| ������  | ����               | ���� | ˵��                                                         |
| :-----: | ------------------ | ---- | ------------------------------------------------------------ |
|   vm    | const EcmaVM *     | ��   | ���������                                                 |
| handler | Local<FunctionRef> | ��   | ָ��FunctionRef���͵ľֲ���������ʾ�����쳣�Ļص�����������Promise�����з����쳣ʱ�����á� |

**����ֵ��**

| ����              | ˵��                                                         |
| ----------------- | ------------------------------------------------------------ |
| Local<PromiseRef> | ����ڵ��ù����з����жϣ��򷵻�δ���壨undefined�������򣬽����ת��ΪPromiseRef���Ͳ����ء� |

**ʾ����**

```C++
Local<JSValueRef> FunCallback(JsiRuntimeCallInfo *info)
{
    EscapeLocalScope scope(info->GetVM());
    return scope.Escape(ArrayRef::New(info->GetVM(), info->GetArgsNumber()));
}
Local<PromiseCapabilityRef> capability = PromiseCapabilityRef::New(vm);
Local<PromiseRef> promise = capability->GetPromise(vm);
Local<FunctionRef> reject = FunctionRef::New(vm, FunCallback);
Local<PromiseRef> result = promise->Catch(vm, reject);
```

### Then

Local<PromiseRef> Then(const EcmaVM *vm, Local<FunctionRef> handler)��

��Promise����һ���ص�������Promise�����ö�ʱִ�еĺ�����

Local<PromiseRef> Then(const EcmaVM *vm, Local<FunctionRef> onFulfilled, Local<FunctionRef> onRejected)��

��Promise����һ���ص�������Promise�����ö�ִ��onFulfilled��Promise����ܾ�ִ��onRejected��

**������**

|   ������    | ����               | ���� | ˵��                        |
| :---------: | ------------------ | ---- | --------------------------- |
|     vm      | const EcmaVM *     | ��   | ���������                |
| onFulfilled | Local<FunctionRef> | ��   | Promise�����ö�ִ�еĺ����� |
| onRejected  | Local<FunctionRef> | ��   | Promise����ܾ�ִ�еĺ����� |

**����ֵ��**

| ����              | ˵��                                                         |
| ----------------- | ------------------------------------------------------------ |
| Local<PromiseRef> | ������Ϊ Local<JSValueRef>���Ͷ��󣬲���Ϊ�����ķ���ֵ�������ж��첽�����Ƿ����óɹ��� |

**ʾ����**

```C++
Local<JSValueRef> FunCallback(JsiRuntimeCallInfo *info)
{
    EscapeLocalScope scope(info->GetVM());
    return scope.Escape(ArrayRef::New(info->GetVM(), info->GetArgsNumber()));
}
Local<PromiseCapabilityRef> capability = PromiseCapabilityRef::New(vm);
Local<PromiseRef> promise = capability->GetPromise(vm);
Local<FunctionRef> callback = FunctionRef::New(vm, FunCallback);
Local<PromiseRef> result = promise->Then(vm, callback, callback);
```

### Finally

Local<PromiseRef> Finally(const EcmaVM *vm, Local<FunctionRef> handler)��

����Promise�����ö����Ǿܾ�����ִ�еĺ�����

**������**

| ������  | ����               | ���� | ˵��             |
| :-----: | ------------------ | ---- | ---------------- |
|   vm    | const EcmaVM *     | ��   | ���������     |
| handler | Local<FunctionRef> | ��   | ��Ҫִ�еĺ����� |

**����ֵ��**

| ����              | ˵��                                                         |
| ----------------- | ------------------------------------------------------------ |
| Local<PromiseRef> | ������Ϊ Local<JSValueRef>���Ͷ��󣬲���Ϊ�����ķ���ֵ�������ж��첽�����Ƿ����óɹ��� |

**ʾ����**

```C++
Local<JSValueRef> FunCallback(JsiRuntimeCallInfo *info)
{
    EscapeLocalScope scope(info->GetVM());
    return scope.Escape(ArrayRef::New(info->GetVM(), info->GetArgsNumber()));
}
Local<PromiseCapabilityRef> capability = PromiseCapabilityRef::New(vm);
Local<PromiseRef> promise = capability->GetPromise(vm);
Local<FunctionRef> callback = FunctionRef::New(vm, FunCallback);
Local<PromiseRef> result = promise->Finally(vm, callback);
```

## TryCatch

�쳣�����࣬������JS�в���ʹ���һЩ�쳣��

### GetAndClearException

Local<ObjectRef> GetAndClearException()��

��ȡ��������񵽵��쳣����

**������**

| ������ | ���� | ���� | ˵�� |
| ------ | ---- | ---- | ---- |
| �޲�   |      |      |      |

**����ֵ��**

| ����             | ˵��                                                         |
| ---------------- | ------------------------------------------------------------ |
| Local<ObjectRef> | ��ȡ���񵽵��쳣��������ת��Ϊ Local<ObjectRef>���Ͷ��󣬲�����Ϊ�����ķ���ֵ�� |

**ʾ����**

```C++
Local<StringRef> message = StringRef::NewFromUtf8(vm, "ErrorTest");
JSNApi::ThrowException(vm, Exception::Error(vm, message););
TryCatch tryCatch(vm);
Local<ObjectRef> error = tryCatch.GetAndClearException();
```

## BigInt64ArrayRef

���ڱ�ʾһ��64λ�������飬��ͨ�����ڴ�����������㣬��Ϊ��ͨ��Number������JavaScript��ֻ�ܾ�ȷ��ʾ��53λ����

### New

static Local<BigInt64ArrayRef> New(const EcmaVM *vm, Local<ArrayBufferRef> buffer, int32_t byteOffset, int32_t length)��

����һ��BigInt64ArrayRef����

**������**

| ������     | ����                  | ���� | ˵��                                  |
| ---------- | --------------------- | ---- | ------------------------------------- |
| vm         | const EcmaVM *        | ��   | ���������                          |
| buffer     | Local<ArrayBufferRef> | ��   | һ�� ArrayBuffer �������ڴ洢���ݡ� |
| byteOffset | int32_t               | ��   | ��ʾ�ӻ��������ĸ��ֽڿ�ʼ��ȡ���ݡ�  |
| length     | int32_t               | ��   | ��ʾҪ��ȡ��Ԫ��������                |

**����ֵ��**

| ����                    | ˵��                           |
| ----------------------- | ------------------------------ |
| Local<BigInt64ArrayRef> | һ���µ�BigInt64ArrayRef���� |

**ʾ����**

```C++
Local<ArrayBufferRef> buffer = ArrayBufferRef::New(vm, 5);
Local<ObjectRef> object = BigInt64ArrayRef::New(vm, buffer, 0, 5);
```

## BigIntRef

���ڱ�ʾ���������������ṩ��һ�ַ�����������Number�����ܱ�ʾ��������Χ�����֡�

### New

static Local<BigIntRef> New(const EcmaVM *vm, int64_t input)��

����һ���µ�BigIntRef����

**������**

| ������ | ����           | ���� | ˵��                          |
| ------ | -------------- | ---- | ----------------------------- |
| vm     | const EcmaVM * | ��   | ���������                  |
| input  | int64_t        | ��   | ��ҪתΪBigIntRef�������ֵ�� |

**����ֵ��**

| ����             | ˵��                    |
| ---------------- | ----------------------- |
| Local<BigIntRef> | һ���µ�BigIntRef���� |

**ʾ����**

```C++
int64_t maxInt64 = std::numeric_limits<int64_t>::max();
Local<BigIntRef> valie = BigIntRef::New(vm, maxInt64);
```

### BigIntToInt64

void BigIntRef::BigIntToInt64(const EcmaVM *vm, int64_t *cValue, bool *lossless)��

��BigInt����ת��Ϊ64λ�з����������Ƿ��ܹ���ȷ��������ת����

**������**

| ������   | ����           | ���� | ˵��                                    |
| -------- | -------------- | ---- | --------------------------------------- |
| vm       | const EcmaVM * | ��   | ���������                            |
| cValue   | int64_t *      | ��   | ���ڴ洢ת��ΪInt64��ֵ�ı�����         |
| lossless | bool *         | ��   | �����жϳ������Ƿ��ܹ�ת��ΪInt64���͡� |

**����ֵ��**

| ���� | ˵��       |
| ---- | ---------- |
| void | �޷���ֵ�� |

**ʾ����**

```C++
uint64_t maxUint64 = std::numeric_limits<uint64_t>::max();
Local<BigIntRef> maxBigintUint64 = BigIntRef::New(vm, maxUint64);
int64_t toNum;
bool lossless = true;
maxBigintUint64->BigIntToInt64(vm, &toNum, &lossless);
```

### BigIntToUint64

void BigIntRef::BigIntToUint64(const EcmaVM *vm, uint64_t *cValue, bool *lossless)��

��BigInt����ת��Ϊ64λ�޷�������������ת���Ƿ������ȷ����

**������**

| ������   | ����           | ���� | ˵��                                    |
| -------- | -------------- | ---- | --------------------------------------- |
| vm       | const EcmaVM * | ��   | ���������                            |
| cValue   | uint64_t *     | ��   | ���ڴ洢ת��Ϊuint64_t��ֵ�ı�����      |
| lossless | bool *         | ��   | �����жϳ������Ƿ��ܹ�ת��ΪInt64���͡� |

**����ֵ��**

| ���� | ˵��       |
| ---- | ---------- |
| void | �޷���ֵ�� |

**ʾ����**

```C++
uint64_t maxUint64 = std::numeric_limits<uint64_t>::max();
Local<BigIntRef> maxBigintUint64 = BigIntRef::New(vm, maxUint64);
int64_t toNum;
bool lossless = true;
maxBigintUint64->BigIntToInt64(vm, &toNum, &lossless);
```

### CreateBigWords

Local<JSValueRef> BigIntRef::CreateBigWords(const EcmaVM *vm, bool sign, uint32_t size, const uint64_t* words)��

��һ��uint64_t�����װΪһ��BigIntRef����

**������**

| ������ | ����            | ���� | ˵��                                 |
| ------ | --------------- | ---- | ------------------------------------ |
| vm     | const EcmaVM *  | ��   | ���������                         |
| sign   | bool            | ��   | ȷ�����ɵ� `BigInt` ���������Ǹ����� |
| size   | uint32_t        | ��   | uint32_t �����С��                  |
| words  | const uint64_t* | ��   | uint32_t ���顣                      |

**����ֵ��**

| ����              | ˵��                                                         |
| ----------------- | ------------------------------------------------------------ |
| Local<JSValueRef> | ��uint32_t ת��ΪBigIntRef���󣬲�����ת��ΪLocal<JSValueRef>���ͣ���Ϊ�����ķ���ֵ�� |

**ʾ����**

```C++
bool sign = false;
uint32_t size = 3;
const uint64_t words[3] = {
    std::numeric_limits<uint64_t>::min() - 1,
    std::numeric_limits<uint64_t>::min(),
    std::numeric_limits<uint64_t>::max(),
};
Local<JSValueRef> bigWords = BigIntRef::CreateBigWords(vm, sign, size, words);
```

### GetWordsArraySize

uint32_t GetWordsArraySize()��

��ȡBigIntRef�����װuint64_t����Ĵ�С��

**������**

| ������ | ���� | ���� | ˵�� |
| ------ | ---- | ---- | ---- |
| �޲�   |      |      |      |

**����ֵ��**

| ����     | ˵��                                      |
| -------- | ----------------------------------------- |
| uint32_t | ����BigIntRef�����װuint64_t����Ĵ�С�� |

**ʾ����**

```C++
bool sign = false;
uint32_t size = 3;
const uint64_t words[3] = {
    std::numeric_limits<uint64_t>::min() - 1,
    std::numeric_limits<uint64_t>::min(),
    std::numeric_limits<uint64_t>::max(),
};
Local<JSValueRef> bigWords = BigIntRef::CreateBigWords(vm, sign, size, words);
Local<BigIntRef> bigWordsRef(bigWords);
uint32_t cnt = bigWordsRef->GetWordsArraySize();
```

## StringRef

�̳���PrimitiveRef�����ڱ�ʾ�ַ����������ݵ����ã��ṩ��һЩ���ַ����Ĳ���������

### NewFromUtf8

Local<StringRef> StringRef::NewFromUtf8(const EcmaVM *vm, const char *utf8, int length)��

����utf8���͵�StringRef����

**������**

| ������ | ����           | ���� | ˵��             |
| ------ | -------------- | ---- | ---------------- |
| vm     | const EcmaVM * | ��   | ���������     |
| utf8   | char *         | ��   | char�����ַ����� |
| int    | length         | ��   | �ַ������ȡ�     |

**����ֵ��**

| ����             | ˵��                    |
| ---------------- | ----------------------- |
| Local<StringRef> | һ���µ�StringRef���� |

**ʾ����**

```C++
std::string testUtf8 = "Hello world";
Local<StringRef> description = StringRef::NewFromUtf8(vm, testUtf8.c_str());
```

### NewFromUtf16

Local<StringRef> StringRef::NewFromUtf16(const EcmaVM *vm, const char16_t *utf16, int length)��

����utf16���͵�StringRef����

**������**

| ������ | ����           | ���� | ˵��                  |
| ------ | -------------- | ---- | --------------------- |
| vm     | const EcmaVM * | ��   | ���������          |
| utf16  | char16_t *     | ��   | char16_t �����ַ����� |
| int    | length         | ��   | �ַ������ȡ�          |

**����ֵ��**

| ����             | ˵��                    |
| ---------------- | ----------------------- |
| Local<StringRef> | һ���µ�StringRef���� |

**ʾ����**

```C++
char16_t data = 0Xdf06;
Local<StringRef> obj = StringRef::NewFromUtf16(vm, &data);
```

### Utf8Length

int32_t StringRef::Utf8Length(const EcmaVM *vm)��

��utf8���Ͷ�ȡStringRef��ֵ���ȡ�

**������**

| ������ | ����           | ���� | ˵��         |
| ------ | -------------- | ---- | ------------ |
| vm     | const EcmaVM * | ��   | ��������� |

**����ֵ��**

| ����    | ˵��                   |
| ------- | ---------------------- |
| int32_t | utf8�����ַ����ĳ��ȡ� |

**ʾ����**

```C++
std::string testUtf8 = "Hello world";
Local<StringRef> stringObj = StringRef::NewFromUtf8(vm, testUtf8.c_str());
int32_t lenght = stringObj->Utf8Length(vm);
```

### WriteUtf8

int StringRef::WriteUtf8(char *buffer, int length, bool isWriteBuffer)��

��StringRef��ֵд��char���黺������

**������**

| ������        | ����   | ���� | ˵��                                  |
| ------------- | ------ | ---- | ------------------------------------- |
| buffer        | char * | ��   | ��Ҫд��Ļ�������                    |
| length        | int    | ��   | ��Ҫд�뻺�����ĳ��ȡ�                |
| isWriteBuffer | bool   | ��   | �Ƿ���Ҫ��StringRef��ֵд�뵽�������� |

**����ֵ��**

| ���� | ˵��                              |
| ---- | --------------------------------- |
| int  | ��StringRef��ֵתΪUtf8��ĳ��ȡ� |

**ʾ����**

```C++
Local<StringRef> local = StringRef::NewFromUtf8(vm, "abcdefbb");
char cs[16] = {0};
int length = local->WriteUtf8(cs, 6);
```

### WriteUtf16

int StringRef::WriteUtf16(char16_t *buffer, int length)��

��StringRef��ֵд��char���黺������

**������**

| ������ | ����   | ���� | ˵��                   |
| ------ | ------ | ---- | ---------------------- |
| buffer | char * | ��   | ��Ҫд��Ļ�������     |
| length | int    | ��   | ��Ҫд�뻺�����ĳ��ȡ� |

**����ֵ��**

| ���� | ˵��                              |
| ---- | --------------------------------- |
| int  | ��StringRef��ֵתΪUtf8��ĳ��ȡ� |

**ʾ����**

```c++
Local<StringRef> local = StringRef::NewFromUtf16(vm, u"���ã���Ϊ��");
char cs[16] = {0};
int length = local->WriteUtf16(cs, 3);
```

### Length

uint32_t StringRef::Length()��

��ȡStringRef��ֵ�ĳ��ȡ�

**������**

| ������ | ���� | ���� | ˵�� |
| ------ | ---- | ---- | ---- |
| �޲�   |      |      |      |

**����ֵ��**

| ���� | ˵��                  |
| ---- | --------------------- |
| int  | StringRef��ֵ�ĳ��ȡ� |

**ʾ����**

```c++
Local<StringRef> local = StringRef::NewFromUtf8(vm, "abcdefbb");
int len = local->Length()
```

### ToString

std::string StringRef::ToString()��

��StringRef��ֵת��Ϊstd::string��

**������**

| ������ | ���� | ���� | ˵�� |
| ------ | ---- | ---- | ---- |
| �޲�   |      |      |      |

**����ֵ��**

| ����        | ˵��                                  |
| ----------- | ------------------------------------- |
| std::string | ��StringRef��valueתΪC++string���͡� |

**ʾ����**

```c++
Local<StringRef> stringObj = StringRef::NewFromUtf8(vm, "abc");
std::string str = stringObj->ToString();
```

## PromiseRejectInfo

`PromiseRejectInfo` �����ڴ洢�й� Promise ���ܾ��¼�����Ϣ���������ܾ��� Promise ���󡢾ܾ���ԭ���¼����ͺ����¼���ص����ݡ��ṩ����Ӧ�ķ��ʷ������ڻ�ȡ��Щ��Ϣ��

### GetPromise

Local<JSValueRef> GetPromise() const��

��ȡһ��Promise����

**������**

| ������ | ���� | ���� | ˵�� |
| ------ | ---- | ---- | ---- |
| �޲�   |      |      |      |

**����ֵ��**

| ����              | ˵��                                                         |
| ----------------- | ------------------------------------------------------------ |
| Local<JSValueRef> | ��ȡPromise���󣬲�����ת��ΪLocal<JSValueRef>���ͣ���Ϊ�����ķ���ֵ�� |

**ʾ����**

```C++
Local<JSValueRef> promise(PromiseCapabilityRef::New(vm)->GetPromise(vm));
Local<StringRef> toStringReason = StringRef::NewFromUtf8(vm, "3.14");
Local<JSValueRef> reason(toStringReason);
void *data = static_cast<void *>(new std::string("promisereject"));
PromiseRejectInfo promisereject(promise, reason, PromiseRejectInfo::PROMISE_REJECTION_EVENT::REJECT, data);
Local<JSValueRef> obj = promisereject.GetPromise();
```

## PromiseCapabilityRef

`PromiseCapabilityRef` ���� `ObjectRef` ������࣬ר�����ڴ��� Promise ����Ĺ��ܡ����ṩ�˴����µ� PromiseCapability ���󡢽�� Promise���ܾ� Promise �Լ���ȡ Promise �ķ�����

### Resolve

bool Resolve(const EcmaVM *vm, Local<JSValueRef> value)��

�����ö�Promise����

**������**

| ������ | ����              | ���� | ˵��                         |
| ------ | ----------------- | ---- | ---------------------------- |
| vm     | const EcmaVM *    | ��   | ���������                 |
| value  | Local<JSValueRef> | ��   | ִ�лص�����������Ҫ�Ĳ����� |

**����ֵ��**

| ���� | ˵��                                |
| ---- | ----------------------------------- |
| bool | Promise����Ļص������Ƿ�ɹ����á� |

**ʾ����**

```c++
Local<JSValueRef> FunCallback(JsiRuntimeCallInfo *info)
{
    EscapeLocalScope scope(info->GetVM());
    return scope.Escape(ArrayRef::New(info->GetVM(), info->GetArgsNumber()));
}
Local<PromiseCapabilityRef> capability = PromiseCapabilityRef::New(vm);
Local<PromiseRef> promise = capability->GetPromise(vm);
promise->Then(vm, FunctionRef::New(vm, FunCallback), FunctionRef::New(vm, FunCallback));
bool b = capability->Resolve(vm, NumberRef::New(vm, 300.3));
```

### Reject

bool Reject(const EcmaVM *vm, Local<JSValueRef> reason)��

���ھܾ�Promise����

**������**

| ������ | ����              | ���� | ˵��                         |
| ------ | ----------------- | ---- | ---------------------------- |
| vm     | const EcmaVM *    | ��   | ���������                 |
| value  | Local<JSValueRef> | ��   | ִ�лص�����������Ҫ�Ĳ����� |

**����ֵ��**

| ���� | ˵��                                |
| ---- | ----------------------------------- |
| bool | Promise����Ļص������Ƿ�ɹ����á� |

**ʾ����**

```C++
Local<JSValueRef> FunCallback(JsiRuntimeCallInfo *info)
{
    EscapeLocalScope scope(info->GetVM());
    return scope.Escape(ArrayRef::New(info->GetVM(), info->GetArgsNumber()));
}
Local<PromiseCapabilityRef> capability = PromiseCapabilityRef::New(vm);
Local<PromiseRef> promise = capability->GetPromise(vm);
promise->Then(vm, FunctionRef::New(vm, FunCallback), FunctionRef::New(vm, FunCallback));
bool b = capability->Reject(vm, NumberRef::New(vm, 300.3));
```
