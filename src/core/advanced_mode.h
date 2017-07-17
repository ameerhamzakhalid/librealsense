// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.
#pragma once

#include "ds5/ds5-private.h"
#include "hw-monitor.h"
#include "streaming.h"
#include "../option.h"
#define RS400_ADVANCED_MODE_HPP
#include "../../rs400/rs400_advanced_mode/src/presets.h"
#include <librealsense/rs2_advanced_mode_command.h>
#undef RS400_ADVANCED_MODE_HPP

typedef enum
{
    etDepthControl              = 0,
    etRsm                       = 1,
    etRauSupportVectorControl   = 2,
    etColorControl              = 3,
    etRauColorThresholdsControl = 4,
    etSloColorThresholdsControl = 5,
    etSloPenaltyControl         = 6,
    etHdad                      = 7,
    etColorCorrection           = 8,
    etDepthTableControl         = 9,
    etAEControl                 = 10,
    etCencusRadius9             = 11,
    etLastAdvancedModeGroup     = 12,       // Must be last
}
EtAdvancedModeRegGroup;

namespace librealsense
{
    template<class T>
    struct advanced_mode_traits;

    #define MAP_ADVANCED_MODE(T, E) template<> struct advanced_mode_traits<T> { static const EtAdvancedModeRegGroup group = E; }

    MAP_ADVANCED_MODE(STDepthControlGroup, etDepthControl);
    MAP_ADVANCED_MODE(STRsm, etRsm);
    MAP_ADVANCED_MODE(STRauSupportVectorControl, etRauSupportVectorControl);
    MAP_ADVANCED_MODE(STColorControl, etColorControl);
    MAP_ADVANCED_MODE(STRauColorThresholdsControl, etRauColorThresholdsControl);
    MAP_ADVANCED_MODE(STSloColorThresholdsControl, etSloColorThresholdsControl);
    MAP_ADVANCED_MODE(STSloPenaltyControl, etSloPenaltyControl);
    MAP_ADVANCED_MODE(STHdad, etHdad);
    MAP_ADVANCED_MODE(STColorCorrection, etColorCorrection);
    MAP_ADVANCED_MODE(STDepthTableControl, etDepthTableControl);
    MAP_ADVANCED_MODE(STAEControl, etAEControl);
    MAP_ADVANCED_MODE(STCensusRadius, etCencusRadius9);


    class ds5_advanced_mode_interface : public virtual device_interface
    {
    public:
        virtual bool is_enabled() const = 0;

        virtual void toggle_advanced_mode(bool enable) = 0;

        virtual void apply_preset(const std::string& pid,
                                  const std::vector<platform::stream_profile>& configuration,
                                  rs2_rs400_visual_preset preset) = 0;

        virtual void get_depth_control_group(STDepthControlGroup* ptr, int mode = 0) const = 0;
        virtual void get_rsm(STRsm* ptr, int mode = 0) const = 0;
        virtual void get_rau_support_vector_control(STRauSupportVectorControl* ptr, int mode = 0) const = 0;
        virtual void get_color_control(STColorControl* ptr, int mode = 0) const = 0;
        virtual void get_rau_color_thresholds_control(STRauColorThresholdsControl* ptr, int mode = 0) const = 0;
        virtual void get_slo_color_thresholds_control(STSloColorThresholdsControl* ptr, int mode = 0) const = 0;
        virtual void get_slo_penalty_control(STSloPenaltyControl* ptr, int mode = 0) const = 0;
        virtual void get_hdad(STHdad* ptr, int mode = 0) const = 0;
        virtual void get_color_correction(STColorCorrection* ptr, int mode = 0) const = 0;
        virtual void get_depth_table_control(STDepthTableControl* ptr, int mode = 0) const = 0;
        virtual void get_ae_control(STAEControl* ptr, int mode = 0) const = 0;
        virtual void get_census_radius(STCensusRadius* ptr, int mode = 0) const = 0;

        virtual void set_depth_control_group(const STDepthControlGroup& val) = 0;
        virtual void set_rsm(const STRsm& val) = 0;
        virtual void set_rau_support_vector_control(const STRauSupportVectorControl& val) = 0;
        virtual void set_color_control(const STColorControl& val) = 0;
        virtual void set_rau_color_thresholds_control(const STRauColorThresholdsControl& val) = 0;
        virtual void set_slo_color_thresholds_control(const STSloColorThresholdsControl& val) = 0;
        virtual void set_slo_penalty_control(const STSloPenaltyControl& val) = 0;
        virtual void set_hdad(const STHdad& val) = 0;
        virtual void set_color_correction(const STColorCorrection& val) = 0;
        virtual void set_depth_table_control(const STDepthTableControl& val) = 0;
        virtual void set_ae_control(const STAEControl& val) = 0;
        virtual void set_census_radius(const STCensusRadius& val) = 0;

        virtual ~ds5_advanced_mode_interface() = default;
    };

    MAP_EXTENSION(RS2_EXTENSION_TYPE_ADVANCED_MODE, librealsense::ds5_advanced_mode_interface);

    class advanced_mode_preset_option;

    class ds5_advanced_mode_base : public ds5_advanced_mode_interface
    {
    public:
        explicit ds5_advanced_mode_base(std::shared_ptr<hw_monitor> hwm, uvc_sensor& depth_sensor);

        bool is_enabled() const;
        void toggle_advanced_mode(bool enable);
        void apply_preset(const std::string& pid,
                          const std::vector<platform::stream_profile>& configuration,
                          rs2_rs400_visual_preset preset);

        void get_depth_control_group(STDepthControlGroup* ptr, int mode = 0) const;
        void get_rsm(STRsm* ptr, int mode = 0) const;
        void get_rau_support_vector_control(STRauSupportVectorControl* ptr, int mode = 0) const;
        void get_color_control(STColorControl* ptr, int mode = 0) const;
        void get_rau_color_thresholds_control(STRauColorThresholdsControl* ptr, int mode = 0) const;
        void get_slo_color_thresholds_control(STSloColorThresholdsControl* ptr, int mode = 0) const;
        void get_slo_penalty_control(STSloPenaltyControl* ptr, int mode = 0) const;
        void get_hdad(STHdad* ptr, int mode = 0) const;
        void get_color_correction(STColorCorrection* ptr, int mode = 0) const;
        void get_depth_table_control(STDepthTableControl* ptr, int mode = 0) const;
        void get_ae_control(STAEControl* ptr, int mode = 0) const;
        void get_census_radius(STCensusRadius* ptr, int mode = 0) const;

        void set_depth_control_group(const STDepthControlGroup& val);
        void set_rsm(const STRsm& val);
        void set_rau_support_vector_control(const STRauSupportVectorControl& val);
        void set_color_control(const STColorControl& val);
        void set_rau_color_thresholds_control(const STRauColorThresholdsControl& val);
        void set_slo_color_thresholds_control(const STSloColorThresholdsControl& val);
        void set_slo_penalty_control(const STSloPenaltyControl& val);
        void set_hdad(const STHdad& val);
        void set_color_correction(const STColorCorrection& val);
        void set_depth_table_control(const STDepthTableControl& val);
        void set_ae_control(const STAEControl& val);
        void set_census_radius(const STCensusRadius& val);

    private:
        std::shared_ptr<hw_monitor> _hw_monitor;
        uvc_sensor& _depth_sensor;
        lazy<bool> _enabled;

        enum res_type{
            small_resolution,
            vga_resolution,
            full_resolution
        };

        static const uint16_t HW_MONITOR_COMMAND_SIZE = 1000;
        static const uint16_t HW_MONITOR_BUFFER_SIZE = 1024;

        std::string pid_to_str(uint16_t pid);
        res_type get_res_type(uint32_t width, uint32_t height);

        preset get_all();
        void set_all(const preset& p);

        std::vector<uint8_t> send_recieve(const std::vector<uint8_t>& input) const;

        template<class T>
        void set(const T& strct, EtAdvancedModeRegGroup cmd) const
        {
            auto ptr = (uint8_t*)(&strct);
            std::vector<uint8_t> data(ptr, ptr + sizeof(T));

            assert_no_error(ds::fw_cmd::set_advanced,
                send_recieve(encode_command(ds::fw_cmd::set_advanced, static_cast<uint32_t>(cmd), 0, 0, 0, data)));
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        template<class T>
        T get(EtAdvancedModeRegGroup cmd, T* ptr = static_cast<T*>(nullptr), int mode = 0) const
        {
            T res;
            auto data = assert_no_error(ds::fw_cmd::get_advanced,
                send_recieve(encode_command(ds::fw_cmd::get_advanced,
                static_cast<uint32_t>(cmd), mode)));
            if (data.size() < sizeof(T))
            {
                throw std::runtime_error("The camera returned invalid sized result!");
            }
            res = *reinterpret_cast<T*>(data.data());
            return res;
        }

        static uint32_t pack(uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3);

        static std::vector<uint8_t> assert_no_error(ds::fw_cmd opcode, const std::vector<uint8_t>& results);

        std::vector<uint8_t> encode_command(ds::fw_cmd opcode,
                                            uint32_t p1 = 0,
                                            uint32_t p2 = 0,
                                            uint32_t p3 = 0,
                                            uint32_t p4 = 0,
                                            std::vector<uint8_t> data = std::vector<uint8_t>()) const;
    };


    class advanced_mode_preset_option : public option_base
    {
    public:
        advanced_mode_preset_option(ds5_advanced_mode_base& advanced, uvc_sensor& ep,
                                    const option_range& opt_range);

        static rs2_rs400_visual_preset to_preset(float x);
        void set(float value) override;
        float query() const override;
        bool is_enabled() const override;
        const char* get_description() const override;
        const char* get_value_description(float val) const override;

    private:
        std::mutex _mtx;
        uvc_sensor& _ep;
        ds5_advanced_mode_base& _advanced;
        rs2_rs400_visual_preset _last_preset;
    };
}