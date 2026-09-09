/* Small native machine boundary.
 *
 * Game code talks to this module through typed memory accessors and named
 * ports. It does not write byte offsets directly. The address values retained
 * here are evidence labels; they are not a claim that every field is fully
 * understood yet.
 */
#ifndef STUNRUN_NATIVE_FAKE_PORTS_H
#define STUNRUN_NATIVE_FAKE_PORTS_H

#include <stddef.h>
#include <stdint.h>

#define STUNRUN_MAIN_RAM_BASE 0xFF8000u
#define STUNRUN_MAIN_RAM_BYTES 0x8000u
#define STUNRUN_FAKE_PORT_COUNT 64u
#define STUNRUN_GSP_FIFO_CAPACITY 1024u

typedef enum stunrun_port_id {
    STUNRUN_PORT_MAIN_IN0 = 0,
    STUNRUN_PORT_MAIN_A800,
    STUNRUN_PORT_AD_STICK_X,
    STUNRUN_PORT_AD_STICK_Y,
    STUNRUN_PORT_COIN_1,
    STUNRUN_PORT_PLAYER_START,
    STUNRUN_PORT_SOUND_COMMAND,
    STUNRUN_PORT_SOUND_RESPONSE,
    STUNRUN_PORT_COUNT
} stunrun_port_id_t;

typedef struct stunrun_fake_ports {
    uint8_t main_ram[STUNRUN_MAIN_RAM_BYTES];
    int32_t values[STUNRUN_PORT_COUNT];
    char named_port[STUNRUN_FAKE_PORT_COUNT][96];
    char named_field[STUNRUN_FAKE_PORT_COUNT][96];
    int32_t named_values[STUNRUN_FAKE_PORT_COUNT];
    size_t named_count;
    uint16_t gsp_fifo[STUNRUN_GSP_FIFO_CAPACITY];
    size_t gsp_fifo_count;
} stunrun_fake_ports_t;

void stunrun_ports_init(stunrun_fake_ports_t *ports);

int stunrun_mem_read8(const stunrun_fake_ports_t *ports, uint32_t address,
                      uint8_t *value);
int stunrun_mem_write8(stunrun_fake_ports_t *ports, uint32_t address,
                       uint8_t value);
int stunrun_mem_read16_be(const stunrun_fake_ports_t *ports, uint32_t address,
                          uint16_t *value);
int stunrun_mem_write16_be(stunrun_fake_ports_t *ports, uint32_t address,
                           uint16_t value);
int stunrun_mem_read32_be(const stunrun_fake_ports_t *ports, uint32_t address,
                          uint32_t *value);
int stunrun_mem_write32_be(stunrun_fake_ports_t *ports, uint32_t address,
                           uint32_t value);

int stunrun_port_read(const stunrun_fake_ports_t *ports, stunrun_port_id_t id,
                      int32_t *value);
int stunrun_port_write(stunrun_fake_ports_t *ports, stunrun_port_id_t id,
                       int32_t value);
int stunrun_port_set_named(stunrun_fake_ports_t *ports, const char *port,
                           const char *field, int32_t value);
int stunrun_port_get_named(const stunrun_fake_ports_t *ports, const char *port,
                           const char *field, int32_t *value);

int stunrun_gsp_fifo_push(stunrun_fake_ports_t *ports, uint16_t value);
int stunrun_gsp_fifo_pop(stunrun_fake_ports_t *ports, uint16_t *value);

#endif
