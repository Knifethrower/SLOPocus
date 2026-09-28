/*
 * Copyright (C) 2023, A. Roldán. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "definitions.h"
#include "slopocus.h"
#include "engine/log.h"

#ifdef _WIN32
#include <windows.h>
#include <cstdio>

// Crash diagnostics for the Windows build: the faulting address and the
// return addresses on the stack, as offsets from the module base so they
// can be resolved with x86_64-w64-mingw32-addr2line -e SLOPocus.exe
// 0x140000000+offset.
static LONG WINAPI crashHandler(EXCEPTION_POINTERS* info) {
	const auto base = (uintptr_t)GetModuleHandleA(nullptr);
	const auto at = (uintptr_t)info->ExceptionRecord->ExceptionAddress;
	LOGE << "CRASH: exception 0x" << std::hex << info->ExceptionRecord->ExceptionCode << " at module+0x" << (at - base);
	void* frames[48];
	const USHORT count = RtlCaptureStackBackTrace(0, 48, frames, nullptr);
	for (USHORT i = 0; i < count; i++) {
		const auto frame = (uintptr_t)frames[i];
		if (frame >= base && frame < base + 0x2000000) {
			LOGE << "CRASH: frame " << std::dec << i << " module+0x" << std::hex << (frame - base);
		}
	}
	fflush(nullptr);
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
	SetUnhandledExceptionFilter(crashHandler);
#endif
	pocus::RendererParameters rendererParameters { SCREEN_WIDTH, SCREEN_HEIGHT, std::string(GAME_NAME), 4.0f };
	SLOPocus sloPocus(rendererParameters);
	return sloPocus.run(argc, argv);
}
