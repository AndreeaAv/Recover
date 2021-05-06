#pragma once
#define BLOCK_SIZE 4096

#define LOG_ERROR(str, ...) fwprintf(stderr, L"ERROR: %s: %s: %d: " str "\n", __FILEW__, \
							  __FUNCTIONW__, __LINE__, __VA_ARGS__)

#ifdef WITH_LOG_INFO
#define LOG_INFO(str, ...) fwprintf(stderr, L"LOG: %s: %s: %d: " str "\n", __FILEW__, \
							  __FUNCTIONW__, __LINE__, __VA_ARGS__)
#else
#define LOG_INFO(fstr, ...)
#endif

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif