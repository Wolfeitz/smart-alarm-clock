#pragma once
void diagnostics_init(void);
void diagnostics_printf(const char *format, ...) __attribute__((format(printf,1,2)));
