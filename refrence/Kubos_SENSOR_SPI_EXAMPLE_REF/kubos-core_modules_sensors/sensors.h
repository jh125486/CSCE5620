/*
 * KubOS Core Flight Services
 * Copyright (C) 2016 Kubos Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SENSORS_H
#define SENSORS_H

/**
 * Sensor status enum
 */
typedef enum
{
    SENSOR_OK = 0,
    SENSOR_ERROR,
    SENSOR_READ_ERROR,
    SENSOR_WRITE_ERROR,
    SENSOR_NOT_FOUND,
    SENSOR_NOT_CALIBRATED
} KSensorStatus;

#endif
