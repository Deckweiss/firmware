#include <avr/pgmspace.h>
#include <stddef.h>
#include "bound_key.h"
#include "keyboard_class.h"

void
BoundKey__set_cell(BoundKey *this, Cell cell)
{
  this->cell = cell;
  this->binding.kind = NOMAP;
}

bool
BoundKey__is_active(BoundKey *this)
{
  return this->cell != DEACTIVATED;
}

void
BoundKey__deactivate(BoundKey *this)
{
  this->cell = DEACTIVATED;
}

static const uint16_t hi_mask  = 0x0F00;
static const uint16_t mid_mask = 0x00F0;
static const uint16_t lo_mask  = 0x000F;

bool exact(uint16_t b, uint16_t p)
{
  uint8_t a =  (b & hi_mask)  >> 8;
  uint8_t br = (b & mid_mask) >> 4;
  uint8_t pr = (p & mid_mask) >> 4;
  uint8_t bl = b & lo_mask;
  uint8_t pl = p & lo_mask;
  b &= ~hi_mask;
  p &= ~hi_mask;
  return ((!b || ((p&b)==b)) && ((((pl&~bl)|(pr&~br)))==a));
}

bool near(uint16_t b, uint16_t p)
{
  uint8_t a =  (b & hi_mask)  >> 8;
  uint8_t br = (b & mid_mask) >> 4;
  uint8_t pr = (p & mid_mask) >> 4;
  uint8_t bl = b & lo_mask;
  uint8_t pl = p & lo_mask;
  b &= ~hi_mask;
  p &= ~hi_mask;
  return ((!b || ((p&b)==b)) && ((((pl&~bl)|(pr&~br))&a)==a));
}

void
BoundKey__update_binding(BoundKey *this, Modifiers mods, KeyMap keymap)
{
  this->binding.kind = NOMAP;

  static KeyBindingArray bindings;
  KeyBindingArray__get(&bindings, &keymap[this->cell]);
  if (bindings.length != 0)
  {
    // find the binding that matches the specified modifier state.
    for (int i = 0; i < bindings.length; ++i)
    {
      const KeyBinding *binding = KeyBindingArray__get_binding(&bindings, i);
      if (exact(binding->premods, mods))
      {
        KeyBinding__copy(binding, &this->binding);
        return;
      }
    }

    // if none match, find one that is close
    for (int i = 0; i < bindings.length; ++i)
    {
      const KeyBinding *binding = KeyBindingArray__get_binding(&bindings, i);
      if (near(binding->premods, mods))
      {
        KeyBinding__copy(binding, &this->binding);
        return;
      }
    }

    // if no match was found, use the default binding
    // TODO: the code generator must ensure that the
    // following assumption is correct, the first
    // binding will be the one and only binding with
    // premods == NONE.
    const KeyBinding *binding = KeyBindingArray__get_binding(&bindings, 0);
    if (binding->premods == NONE)
    {
      KeyBinding__copy(binding, &this->binding);
      return;
    }
  }
}

