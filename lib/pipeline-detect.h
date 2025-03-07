#pragma once
#include <memory>
#include <pipeline-itf.h>
#include <vector>

namespace snowboy {
	struct MatrixBase;
	struct FrameInfo;

	struct InterceptStream;
	struct GainControlStream;
	struct FrontendStream;
	struct FramerStream;
	struct RawEnergyVadStream;
	struct VadStateStream;
	struct FftStream;
	struct MfccStream;
	struct RawNnetVadStream;
	struct EavesdropStream;
	struct InterceptStream;
	struct NnetStream;
	struct TemplateDetectStream;
	struct UniversalDetectStream;

	struct GainControlStreamOptions;
	struct FrontendStreamOptions;
	struct FramerStreamOptions;
	struct RawEnergyVadStreamOptions;
	struct VadStateStreamOptions;
	struct FftStreamOptions;
	struct MfccStreamOptions;
	struct RawNnetVadStreamOptions;
	struct NnetStreamOptions;
	struct TemplateDetectStreamOptions;
	struct UniversalDetectStreamOptions;

	struct PipelineDetectOptions {
		int sampleRate;
		bool applyFrontend;
		// Padding
		void Register(const std::string&, OptionsItf*);
	};
	static_assert(sizeof(PipelineDetectOptions) == 8);

	struct PipelineDetect : PipelineItf {
		// Virtual stuff
		virtual void RegisterOptions(const std::string&, OptionsItf*) override;
		virtual int GetPipelineSampleRate() const override;
		virtual bool Init() override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual std::string OptionPrefix() const override;
		virtual ~PipelineDetect();

		PipelineDetect(const PipelineDetectOptions& options);

		void ApplyFrontend(bool apply);
		void ClassifyModels(const std::string&, std::string*, std::string*);
		void ClassifySensitivities(const std::string&, std::string*, std::string*) const;
		uint64_t GetDetectedFrameId() const;
		std::string GetSensitivity() const;
		int NumHotwords() const;
		int RunDetection(const MatrixBase& data, bool is_end);
		void SetAudioGain(float gain);
		void SetHighSensitivity(const std::string&);
		void SetMaxAudioAmplitude(float maxAmplitude);
		void SetModel(const std::string& model);
		void SetSensitivity(const std::string& sensitivity);
		void UpdateModel() const;

		std::unique_ptr<InterceptStream> m_interceptStream;
		std::unique_ptr<GainControlStream> m_gainControlStream;
		std::unique_ptr<FrontendStream> m_frontendStream;
		std::unique_ptr<FramerStream> m_framerStream;
		std::unique_ptr<RawEnergyVadStream> m_rawEnergyVadStream;
		std::unique_ptr<VadStateStream> m_vadStateStream;
		std::unique_ptr<FftStream> m_fftStream;
		std::unique_ptr<MfccStream> m_mfccStream;
		std::unique_ptr<RawNnetVadStream> m_rawNnetVadStream;
		std::unique_ptr<VadStateStream> m_vadStateStream2;
		std::unique_ptr<EavesdropStream> m_eavesdropStream;

		std::unique_ptr<InterceptStream> m_templateDetectInterceptStream;
		std::unique_ptr<NnetStream> m_templateDetectNnetStream;
		std::unique_ptr<TemplateDetectStream> m_templateDetectStream;

		std::unique_ptr<InterceptStream> m_universalDetectInterceptStream;
		std::unique_ptr<UniversalDetectStream> m_universalDetectStream;

		PipelineDetectOptions m_pipelineDetectOptions = {};
		std::unique_ptr<GainControlStreamOptions> m_gainControlStreamOptions;
		std::unique_ptr<FrontendStreamOptions> m_frontendStreamOptions;
		std::unique_ptr<FramerStreamOptions> m_framerStreamOptions;
		std::unique_ptr<RawEnergyVadStreamOptions> m_rawEnergyVadStreamOptions;
		std::unique_ptr<VadStateStreamOptions> m_vadStateStreamOptions;
		std::unique_ptr<FftStreamOptions> m_fftStreamOptions;
		std::unique_ptr<MfccStreamOptions> m_mfccStreamOptions;
		std::unique_ptr<RawNnetVadStreamOptions> m_rawNnetVadStreamOptions;
		std::unique_ptr<VadStateStreamOptions> m_vadStateStream2Options;
		std::unique_ptr<NnetStreamOptions> m_templateDetectNnetStreamOptions;
		std::unique_ptr<TemplateDetectStreamOptions> m_templateDetectStreamOptions;
		std::unique_ptr<UniversalDetectStreamOptions> m_universalDetectStreamOptions;

		std::vector<FrameInfo> m_eavesdropStreamFrameInfoVector;
		std::vector<bool> field_x110;
		std::vector<int> field_x138;
		std::vector<int> field_x150;

		bool field_x168 = false;
		bool field_x169 = false;
		char data2[6];
	};
	static_assert(sizeof(PipelineDetect) == 368);
	static_assert(sizeof(PipelineDetect::m_eavesdropStreamFrameInfoVector) == 24);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
	static_assert(offsetof(PipelineDetect, m_universalDetectStreamOptions) == 0xf0);
	static_assert(offsetof(PipelineDetect, m_eavesdropStreamFrameInfoVector) == 0xf8);
#pragma GCC diagnostic pop
} // namespace snowboy