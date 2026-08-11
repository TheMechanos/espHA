
#include <cstring>

#include "HASerializerArray.h"
#include "HADictionary.h"

HASerializerArray::HASerializerArray(const uint8_t size, const bool progmemItems) :
    _progmemItems(progmemItems),
    _size(size),
    _itemsNb(0),
    _items(new ItemType[size])
{

}

HASerializerArray::~HASerializerArray()
{
    delete[] _items;
}

bool HASerializerArray::add(ItemType item)
{
    if (_itemsNb >= _size) {
        return false;
    }

    _items[_itemsNb++] = item;
    return true;
}

const char* HASerializerArray::getItem(const uint8_t index) const
{
    if (index >= _itemsNb) {
        return nullptr;
    }

    return _items[index];
}

uint16_t HASerializerArray::calculateSize() const
{
    uint16_t size =
        strlen(HASerializerJsonArrayPrefix) +
        strlen(HASerializerJsonArraySuffix);

    if (_itemsNb == 0) {
        return size;
    }

    // separators between elements
    size += (_itemsNb - 1) * strlen(HASerializerJsonPropertiesSeparator);

    for (uint8_t i = 0; i < _itemsNb; i++) {
        size +=
            2 * strlen(HASerializerJsonEscapeChar)
            + (_progmemItems ? strlen(_items[i]) : strlen(_items[i]));
    }

    return size;
}

bool HASerializerArray::serialize(char* output) const
{
    if (!output) {
        return false;
    }

    strcat(output, HASerializerJsonArrayPrefix);

    for (uint8_t i = 0; i < _itemsNb; i++) {
        if (i > 0) {
            strcat(output, HASerializerJsonPropertiesSeparator);
        }

        strcat(output, HASerializerJsonEscapeChar);

        if (_progmemItems) {
            strcat(output, _items[i]);
        } else {
            strcat(output, _items[i]);
        }

        strcat(output, HASerializerJsonEscapeChar);
    }

    strcat(output, HASerializerJsonArraySuffix);
    return true;
}

void HASerializerArray::clear()
{
    _itemsNb = 0;
}
