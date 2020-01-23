// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.

#ifndef PCH_H
#define PCH_H

#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <utility>
#include <memory>
#include <unordered_map>
#include <set>
#include <vector>
#include <iostream>
#include <functional>
#include <fstream>
#include <assert.h>
#include <mutex>

#include <d3d11_2.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include "common.h"


using std::set;
using std::unordered_map;
using std::string;
using std::vector;
using std::shared_ptr;
using std::unique_ptr;
using std::cout;



// TODO: add headers that you want to pre-compile here

#endif //PCH_H
