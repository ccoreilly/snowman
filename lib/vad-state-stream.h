#pragma once
#include <matrix-wrapper.h>
#include <memory>
#include <stream-itf.h>

namespace snowboy {
	struct VadState;
	struct OptionsItf;
	struct VadStateStreamOptions {
		int min_non_voice_frames;
		int min_voice_frames;
		bool remove_non_voice;
		int extra_frame_adjust;
		void Register(const std::string&, OptionsItf*);
	};
	static_assert(sizeof(VadStateStreamOptions) == 0x10);
	struct VadStateStream : StreamItf {
		const VadStateStreamOptions m_options;
		int field_x28;
		unsigned int field_x2c;
		bool field_x30;
		Matrix m_someMatrix;
		std::vector<FrameInfo> field_x50;
		Matrix m_someOtherMatrix;
		std::vector<FrameInfo> field_x80;
		const std::unique_ptr<VadState> m_vadstate;
		int field_xa0;
		int field_xa4;

		int ProcessCachedSignal(Matrix*, std::vector<FrameInfo>*);
		int ProcessDataAndInfo(const MatrixBase&, const std::vector<FrameInfo>&, Matrix*, std::vector<FrameInfo>*);
		void PrintVlog(SnowboySignal, const std::vector<FrameInfo>&) const;

		VadStateStream(const VadStateStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~VadStateStream();
	};
	static_assert(sizeof(VadStateStream) == 0xa8);
} // namespace snowboy