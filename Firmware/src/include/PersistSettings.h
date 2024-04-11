/* SPDX-License-Identifier: MIT
 * Copyright(c) 2022 Lincoln Lavoie <lincoln.lavoie@gmail.com>
 */

#include <Preferences.h>

#ifndef __PersistSettings__
#define __PersistSettings__

template <class T>
class PersistSettings{
    private:
        bool mValid;
        unsigned int mConfigVersion;
        uint16_t CRC16(byte *data, size_t data_len);
    public:
        T Config;
        PersistSettings(unsigned int ConfigVersion);
        void Begin(void);
        void Write(void);
        void ResetToDefault(void);
        bool Valid(void);
};
#endif

// PersistSettings Constructor
// Use the Config structure as the class template for the object.
// Provide a default version, preferable as a static member of the 
// config object.
template <class T>
PersistSettings<T>::PersistSettings(unsigned int version){
    mConfigVersion = version;
    mValid= false;
}

// Initialize the settings object, read the config from the
// persistent storage, and validate data.  If the config object
// can not be read, an error is detected, or the version mismatches,
// the Config object will be reset to the default values.
template <class T>
void PersistSettings<T>::Begin(void){
    Preferences pref;

    // Setup the preferences namespace, as read only
    if( !pref.begin("PersistSettings", true) ){
        log_e("Failed to open PersistSettings namespace, resetting and writing defaults.\r\n");
        this->ResetToDefault();
        return;
    }

    // Get the stored config version version, if it's not equal to then the version provided
    // with the the detail, we assume the DefaultConfig has a newer revision of the
    // Config object and we initialize to that value instead of stored values.
    // A value of 0 indicates the key didn't exist, assume no settings exist.
    unsigned int storedVersion = pref.getUInt("version", 0);
    if( storedVersion == 0 || storedVersion != mConfigVersion ){
        log_e("Version mismatch (saved: %d, current: %d), resetting to defaults.\r\n", storedVersion, mConfigVersion);
        this->ResetToDefault();
        return;
    }

    // Read the settings from the persistent storage.
    size_t configLen = pref.getBytesLength("config");
    byte memBytes[configLen];
    if (sizeof(T)+2 != configLen ){
        log_e("Length read failure (read: %d, expected: %d), resetting to defaults.\r\n", configLen, sizeof(T)+2);
        this->ResetToDefault();
        return;
    }
    pref.getBytes("config", memBytes, configLen);

    // Check the CRC16
    if( CRC16(memBytes, configLen) ){
        log_e("CRC16 check failed, resetting to default\r\n");
        this->ResetToDefault();
        return;
    }
    
    // All checks passed, valid config, copy the data to the object.
    memcpy(&Config, memBytes, sizeof(T));

    // Close the preferences
    pref.end();

    // Indicate the config is valid
    mValid = true;
}

// Indicates the Config data object is valid.
template <class T>
bool PersistSettings<T>::Valid(void){ return mValid; }

// Resets the Config object to the default value and provided during the 
// construction of the PersistSettings object and writes those values to
// the persistent storage.
template <class T>
void PersistSettings<T>::ResetToDefault(void){
    T newConfig;
    Config = newConfig;
    this->Write();
    mValid = true;
}

// Write the current values of the Config object to persistent storage.
template <class T>
void PersistSettings<T>::Write(void){
    Preferences pref;

    // Setup the preferences namespace, as read only
    pref.begin("PersistSettings", false);

    // Save the version
    pref.putUInt("version", mConfigVersion);

    // Save the config, as a byte array, with 2-byte CRC on the end.
    byte memBytes[sizeof(T)+2];
    memcpy(memBytes, &Config, sizeof(T));
    uint16_t crc = CRC16(memBytes, sizeof(T));
    memBytes[sizeof(T)] = static_cast<byte>((crc&0xFF));
    memBytes[sizeof(T)+1] = static_cast<byte>((crc>>8));
    pref.putBytes("config", memBytes, sizeof(T)+2);

    // Close the preferences
    pref.end();
}

// Calculate the CRC16 over the provided data object (byte array) and
// return the value.
template <class T>
uint16_t PersistSettings<T>::CRC16(byte *data, size_t data_len){
  uint16_t crc = 0xFFFF;
  for(unsigned int i = 0; i < data_len; i++){
    crc ^= data[i];
    for(unsigned char k = 0; k<8; ++k){
      if(crc & 1) crc = (crc >> 1) ^ 0x8005;
      else crc>>=1;
    }
  }
  return crc;
}