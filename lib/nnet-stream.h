#pragma once
#include <memory>
#include <stream-itf.h>
#include <string>

namespace snowboy {
	struct OptionsItf;
	struct Nnet;
	struct NnetStreamOptions {
		std::string model_filename;
		bool pad_context;
		void Register(const std::string&, OptionsItf*);
	};
	static_assert(sizeof(NnetStreamOptions) == 0x10);
	struct NnetStream : StreamItf {
		NnetStreamOptions m_options;
		std::unique_ptr<Nnet> m_nnet;

		NnetStream(const NnetStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~NnetStream();
	};
	static_assert(sizeof(NnetStream) == 0x30);
} // namespace snowboy