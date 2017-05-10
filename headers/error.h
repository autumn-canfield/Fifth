#pragma once
#include<stdlib.h>
#include<stdarg.h>
#include<stdio.h>

static void error(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	exit(1);
}

