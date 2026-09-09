/* See fake_ports.h for the native machine-boundary contract. */
#include "fake_ports.h"

#include <string.h>

static int valid_range(uint32_t address, size_t width)
{
    uint32_t offset;
    if (address < STUNRUN_MAIN_RAM_BASE)
        return 0;
    offset = address - STUNRUN_MAIN_RAM_BASE;
    return offset <= STUNRUN_MAIN_RAM_BYTES &&
           width <= STUNRUN_MAIN_RAM_BYTES - offset;
}

void stunrun_ports_init(stunrun_fake_ports_t *ports)
{
    if (ports == NULL)
        return;
    memset(ports, 0, sizeof(*ports));
    ports->values[STUNRUN_PORT_AD_STICK_X] = 128;
    ports->values[STUNRUN_PORT_AD_STICK_Y] = 128;
}

int stunrun_mem_read8(const stunrun_fake_ports_t *ports, uint32_t address,
                      uint8_t *value)
{
    if (ports == NULL || value == NULL || !valid_range(address, 1u))
        return 0;
    *value = ports->main_ram[address - STUNRUN_MAIN_RAM_BASE];
    return 1;
}

int stunrun_mem_write8(stunrun_fake_ports_t *ports, uint32_t address,
                       uint8_t value)
{
    if (ports == NULL || !valid_range(address, 1u))
        return 0;
    ports->main_ram[address - STUNRUN_MAIN_RAM_BASE] = value;
    return 1;
}

int stunrun_mem_read16_be(const stunrun_fake_ports_t *ports, uint32_t address,
                          uint16_t *value)
{
    uint8_t high, low;
    if (value == NULL || !stunrun_mem_read8(ports, address, &high) ||
        !stunrun_mem_read8(ports, address + 1u, &low))
        return 0;
    *value = (uint16_t)(((uint16_t)high << 8) | low);
    return 1;
}

int stunrun_mem_write16_be(stunrun_fake_ports_t *ports, uint32_t address,
                           uint16_t value)
{
    return stunrun_mem_write8(ports, address, (uint8_t)(value >> 8)) &&
           stunrun_mem_write8(ports, address + 1u, (uint8_t)value);
}

int stunrun_mem_read32_be(const stunrun_fake_ports_t *ports, uint32_t address,
                          uint32_t *value)
{
    uint16_t high, low;
    if (value == NULL || !stunrun_mem_read16_be(ports, address, &high) ||
        !stunrun_mem_read16_be(ports, address + 2u, &low))
        return 0;
    *value = ((uint32_t)high << 16) | low;
    return 1;
}

int stunrun_mem_write32_be(stunrun_fake_ports_t *ports, uint32_t address,
                           uint32_t value)
{
    return stunrun_mem_write16_be(ports, address, (uint16_t)(value >> 16)) &&
           stunrun_mem_write16_be(ports, address + 2u, (uint16_t)value);
}

int stunrun_port_read(const stunrun_fake_ports_t *ports, stunrun_port_id_t id,
                      int32_t *value)
{
    if (ports == NULL || value == NULL || id < 0 || id >= STUNRUN_PORT_COUNT)
        return 0;
    *value = ports->values[id];
    return 1;
}

int stunrun_port_write(stunrun_fake_ports_t *ports, stunrun_port_id_t id,
                       int32_t value)
{
    if (ports == NULL || id < 0 || id >= STUNRUN_PORT_COUNT)
        return 0;
    ports->values[id] = value;
    return 1;
}

static stunrun_port_id_t named_port_id(const char *port, const char *field)
{
    stunrun_port_id_t id;
    if (port == NULL || field == NULL)
        return STUNRUN_PORT_COUNT;
    id = STUNRUN_PORT_COUNT;
    if (strcmp(port, ":mainpcb:8BADC.0") == 0 &&
        strcmp(field, "AD Stick X") == 0)
        id = STUNRUN_PORT_AD_STICK_X;
    else if (strcmp(port, ":mainpcb:8BADC.2") == 0 &&
             strcmp(field, "AD Stick Y") == 0)
        id = STUNRUN_PORT_AD_STICK_Y;
    else if (strcmp(port, ":mainpcb:IN0") == 0 &&
             strcmp(field, "Coin 1") == 0)
        id = STUNRUN_PORT_COIN_1;
    else if (strcmp(port, ":mainpcb:IN0") == 0 &&
             strcmp(field, "1 Player Start") == 0)
        id = STUNRUN_PORT_PLAYER_START;
    else
        return STUNRUN_PORT_COUNT;
    return id;
}

static size_t named_index(const stunrun_fake_ports_t *ports,
                          const char *port, const char *field)
{
    size_t i;
    for (i = 0u; i < ports->named_count; i++)
        if (strcmp(ports->named_port[i], port) == 0 &&
            strcmp(ports->named_field[i], field) == 0)
            return i;
    return STUNRUN_FAKE_PORT_COUNT;
}

int stunrun_port_set_named(stunrun_fake_ports_t *ports, const char *port,
                           const char *field, int32_t value)
{
    stunrun_port_id_t id = named_port_id(port, field);
    size_t index;
    if (ports == NULL || id >= STUNRUN_PORT_COUNT)
        goto generic;
    return stunrun_port_write(ports, id, value);

generic:
    if (ports == NULL || port == NULL || field == NULL)
        return 0;
    index = named_index(ports, port, field);
    if (index == STUNRUN_FAKE_PORT_COUNT) {
        if (ports->named_count >= STUNRUN_FAKE_PORT_COUNT ||
            strlen(port) >= sizeof(ports->named_port[0]) ||
            strlen(field) >= sizeof(ports->named_field[0]))
            return 0;
        index = ports->named_count++;
        strcpy(ports->named_port[index], port);
        strcpy(ports->named_field[index], field);
    }
    ports->named_values[index] = value;
    return 1;
}

int stunrun_port_get_named(const stunrun_fake_ports_t *ports, const char *port,
                           const char *field, int32_t *value)
{
    stunrun_port_id_t id = named_port_id(port, field);
    size_t index;
    if (id >= STUNRUN_PORT_COUNT)
        goto generic;
    return stunrun_port_read(ports, id, value);

generic:
    if (ports == NULL || value == NULL || port == NULL || field == NULL)
        return 0;
    index = named_index(ports, port, field);
    if (index == STUNRUN_FAKE_PORT_COUNT)
        return 0;
    *value = ports->named_values[index];
    return 1;
}

int stunrun_gsp_fifo_push(stunrun_fake_ports_t *ports, uint16_t value)
{
    if (ports == NULL || ports->gsp_fifo_count >= STUNRUN_GSP_FIFO_CAPACITY)
        return 0;
    ports->gsp_fifo[ports->gsp_fifo_count++] = value;
    return 1;
}

int stunrun_gsp_fifo_pop(stunrun_fake_ports_t *ports, uint16_t *value)
{
    size_t i;
    if (ports == NULL || value == NULL || ports->gsp_fifo_count == 0u)
        return 0;
    *value = ports->gsp_fifo[0];
    for (i = 1u; i < ports->gsp_fifo_count; i++)
        ports->gsp_fifo[i - 1u] = ports->gsp_fifo[i];
    ports->gsp_fifo_count--;
    return 1;
}
