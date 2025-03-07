#pragma once
#include <cmath>
#include <cstdint>
#include <iosfwd>
#include <matrix-wrapper.h>
#include <vector-wrapper.h>
#include <vector>

namespace snowboy {
	struct MatrixBase;
	// TODO: This is kaldi::ChunkInfo
	class ChunkInfo {
		int32_t m_feat_dim;
		int32_t m_num_chunks;
		int32_t m_first_offset;
		int32_t m_last_offset;
		std::vector<int32_t> m_offsets;
		friend std::ostream& operator<<(std::ostream& os, const ChunkInfo& e);

	public:
		ChunkInfo() // default constructor we assume this object will not be used
			: m_feat_dim(0), m_num_chunks(0),
			  m_first_offset(0), m_last_offset(0),
			  m_offsets() {}

		ChunkInfo(int32_t feat_dim, int32_t num_chunks,
				  int32_t first_offset, int32_t last_offset)
			: m_feat_dim(feat_dim), m_num_chunks(num_chunks),
			  m_first_offset(first_offset), m_last_offset(last_offset),
			  m_offsets() { Check(); }

		ChunkInfo(int32_t feat_dim, int32_t num_chunks,
				  const std::vector<int32_t> offsets)
			: m_feat_dim(feat_dim), m_num_chunks(num_chunks),
			  m_first_offset(offsets.front()), m_last_offset(offsets.back()),
			  m_offsets(offsets) {
			if (m_last_offset - m_first_offset + 1 == m_offsets.size())
				m_offsets.clear();
			Check();
		}

		int32_t GetIndex(int32_t offset) const;
		int32_t GetOffset(int32_t index) const;

		void CheckSize(const MatrixBase&) const;
		void Check() const;

		// Not in snowboy
		int32_t NumRows() const { return m_num_chunks * (!m_offsets.empty() ? m_offsets.size() : m_last_offset - m_first_offset + 1); }
		int32_t NumCols() const { return m_feat_dim; }
		int32_t NumChunks() const { return m_num_chunks; }
		int32_t ChunkSize() const { return NumRows() / m_num_chunks; }
		void MakeOffsetsContiguous() {
			m_offsets.clear();
			Check();
		}
	};
	static_assert(sizeof(ChunkInfo) == 0x28);
	std::ostream& operator<<(std::ostream& os, const ChunkInfo& e);

	class Component {
	public:
		Component() : m_index(-1) {}

		virtual std::string Type() const = 0;
		virtual int32_t Index() const;
		virtual void SetIndex(int32_t index);
		virtual int32_t InputDim() const = 0;
		virtual int32_t OutputDim() const = 0;
		virtual std::vector<int32_t> Context() const;
		virtual bool HasDataRearragement() const;
		virtual void Propagate(const ChunkInfo& in_info,
							   const ChunkInfo& out_info,
							   const MatrixBase& in,
							   MatrixBase* out) const = 0;

		virtual void Read(bool binary, std::istream* is) = 0;
		virtual void Write(bool binary, std::ostream* os) const = 0;
		virtual Component* Copy() const = 0;
		virtual ~Component() {}

		static Component* NewComponentOfType(const std::string& type);
		static Component* ReadNew(bool binary, std::istream* is);

	private:
		int32_t m_index;
		Component(const Component&) = delete;
		Component& operator=(const Component&) = delete;
		Component(Component&&) = delete;
		Component& operator=(Component&&) = delete;
	};
	static_assert(sizeof(Component) == 0x10); // Actually 0xc, but padding....

	class AffineComponent : public Component {
		bool m_is_gradient = 1;
		Matrix m_linear_params;
		Vector m_bias_params;

	public:
		virtual std::string Type() const override;
		virtual int32_t InputDim() const override;
		virtual int32_t OutputDim() const override;
		virtual void Propagate(const ChunkInfo& in_info,
							   const ChunkInfo& out_info,
							   const MatrixBase& in,
							   MatrixBase* out) const override;

		virtual void Read(bool binary, std::istream* is) override;
		virtual void Write(bool binary, std::ostream* os) const override;
		virtual Component* Copy() const override;
		virtual ~AffineComponent() {}
	};
	static_assert(sizeof(AffineComponent) == 0x38); // m_is_gradient is inside the padding of Component

	class CmvnComponent : public Component {
		bool field_xc = 0;
		Vector m_scales;
		Vector m_offsets;

	public:
		virtual std::string Type() const override;
		virtual int32_t InputDim() const override;
		virtual int32_t OutputDim() const override;
		virtual void Propagate(const ChunkInfo& in_info,
							   const ChunkInfo& out_info,
							   const MatrixBase& in,
							   MatrixBase* out) const override;

		virtual void Read(bool binary, std::istream* is) override;
		virtual void Write(bool binary, std::ostream* os) const override;
		virtual Component* Copy() const override;
		virtual ~CmvnComponent() {}
	};
	static_assert(sizeof(CmvnComponent) == 0x30); // field_xc is inside the padding of Component

	class NormalizeComponent : public Component {
		int32_t m_dim = 0;
		bool field_x10 = 0;
		float field_x14 = pow(2.0, -66);

	public:
		virtual std::string Type() const override;
		virtual int32_t InputDim() const override;
		virtual int32_t OutputDim() const override;
		virtual void Propagate(const ChunkInfo& in_info,
							   const ChunkInfo& out_info,
							   const MatrixBase& in,
							   MatrixBase* out) const override;

		virtual void Read(bool binary, std::istream* is) override;
		virtual void Write(bool binary, std::ostream* os) const override;
		virtual Component* Copy() const override;
		virtual ~NormalizeComponent() {}
	};
	static_assert(sizeof(NormalizeComponent) == 0x18); // m_dim is inside the padding of Component

	class PosteriorMapComponent : public Component {
		bool field_xc;
		int32_t m_inputDim;
		int32_t m_outputDim;
		std::vector<std::vector<int>> m_indices;

	public:
		virtual std::string Type() const override;
		virtual int32_t InputDim() const override;
		virtual int32_t OutputDim() const override;
		virtual void Propagate(const ChunkInfo& in_info,
							   const ChunkInfo& out_info,
							   const MatrixBase& in,
							   MatrixBase* out) const override;

		virtual void Read(bool binary, std::istream* is) override;
		virtual void Write(bool binary, std::ostream* os) const override;
		virtual Component* Copy() const override;
		virtual ~PosteriorMapComponent() {}
	};
	static_assert(sizeof(PosteriorMapComponent) == 0x30); // field_xc is inside the padding of Component

	class RectifiedLinearComponent : public Component {
		int32_t m_dim;
		bool field_x10;

	public:
		virtual std::string Type() const override;
		virtual int32_t InputDim() const override;
		virtual int32_t OutputDim() const override;
		virtual void Propagate(const ChunkInfo& in_info,
							   const ChunkInfo& out_info,
							   const MatrixBase& in,
							   MatrixBase* out) const override;

		virtual void Read(bool binary, std::istream* is) override;
		virtual void Write(bool binary, std::ostream* os) const override;
		virtual Component* Copy() const override;
		virtual ~RectifiedLinearComponent() {}
	};
	static_assert(sizeof(RectifiedLinearComponent) == 0x18); // m_dim is inside the padding of Component

	class SoftmaxComponent : public Component {
		int32_t m_dim;
		bool field_x10;

	public:
		virtual std::string Type() const override;
		virtual int32_t InputDim() const override;
		virtual int32_t OutputDim() const override;
		virtual void Propagate(const ChunkInfo& in_info,
							   const ChunkInfo& out_info,
							   const MatrixBase& in,
							   MatrixBase* out) const override;

		virtual void Read(bool binary, std::istream* is) override;
		virtual void Write(bool binary, std::ostream* os) const override;
		virtual Component* Copy() const override;
		virtual ~SoftmaxComponent() {}
	};
	static_assert(sizeof(SoftmaxComponent) == 0x18); // field_xc is inside the padding of Component

	class SpliceComponent : public Component {
		bool field_xc;
		int m_inputDim;
		int m_constComponentDim;
		std::vector<int> m_context;

	public:
		virtual std::string Type() const override;
		virtual int32_t InputDim() const override;
		virtual int32_t OutputDim() const override;
		virtual std::vector<int32_t> Context() const override;
		virtual bool HasDataRearragement() const override;
		virtual void Propagate(const ChunkInfo& in_info,
							   const ChunkInfo& out_info,
							   const MatrixBase& in,
							   MatrixBase* out) const override;

		virtual void Read(bool binary, std::istream* is) override;
		virtual void Write(bool binary, std::ostream* os) const override;
		virtual Component* Copy() const override;
		virtual ~SpliceComponent() {}
	};
	static_assert(sizeof(SpliceComponent) == 0x30); // field_xc is inside the padding of Component
} // namespace snowboy