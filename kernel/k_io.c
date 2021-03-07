#include "k_io.h"


// void k_outportb(uint16_t port, uint8_t value)
// {
//     asm volatile
//     (
//         "mov dx, %[port]\n"
//         "mov al, %[value]\n"
//         "out dx, al\n"
//         : : [port] "m" (port), [value] "m" (value) : "dx", "al"
//     );
// }

// void k_outportw(uint16_t port, uint16_t value)
// {
//     asm volatile
//     (
//         "mov dx, %[port]\n"
//         "mov ax, %[value]\n"
//         "out dx, ax\n"
//         : : [port] "m" (port), [value] "m" (value) : "dx", "ax"
//     );
// }

// void k_outportd(uint16_t port, uint32_t value)
// {
//     asm volatile
//     (
//         "mov dx, %[port]\n"
//         "mov eax, %[value]\n"
//         "out dx, eax\n"
//         : : [port] "m" (port), [value] "m" (value) : "dx", "eax"
//     );
// }

// uint8_t k_inportb(uint16_t port)
// {
//     uint8_t value;

//     asm volatile 
//     (
//         "mov dx, %[port]\n"
//         "in al, dx\n"
//         "mov %[value], al\n"
//         : [value] "=m" (value) : [port] "m" (port) : "dx", "al"
//     );

//     return value;
// }

// uint16_t k_inportw(uint16_t port)
// {
//     uint16_t value;

//     asm volatile 
//     (
//         "mov dx, %[port]\n"
//         "in ax, dx\n"
//         "mov %[value], ax\n"
//         : [value] "=m" (value) : [port] "m" (port) : "dx", "ax"
//     );

//     return value;
// }

// uint32_t k_inportd(uint16_t port)
// {
//     uint32_t value;

//     asm volatile 
//     (
//         "mov dx, %[port]\n"
//         "in eax, dx\n"
//         "mov %[value], eax\n"
//         : [value] "=m" (value) : [port] "m" (port) : "dx", "eax"
//     );

//     return value;
// }