#pragma once

#include <span>
#include <string>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <memory>
#include <deque>
#include <charconv>

#define FMT_HEADER_ONLY
#include "fmt/format.h"

#include "libdwgr.h"
#include "libdxfrw.h"

#include "adapter.h"

namespace gtl::shape {

	using namespace std::literals;

	template < typename T >
	using ptr_deque = std::deque<std::unique_ptr<T>>;

	class DRW_ImagePath : public DRW_Image {
	public:
		using base_t = DRW_Image;

		using base_t::base_t;

		DRW_ImagePath() = default;
		DRW_ImagePath(DRW_ImagePath const&) = default;
		DRW_ImagePath(DRW_Image const& v) : base_t(v) {}

		std::string path; //stores the image path

		template < typename Archive >
		friend Archive& operator << (Archive& ar, DRW_ImagePath const& v) {
			ar << (DRW_Image const&)v;

			ar.Write("path"sv, v.path);

			return ar;
		}
	};

	class DRW_Entities : public DRW_Block {
	public:
		using base_t = DRW_Block;

		using base_t::base_t;

		DRW_Entities() = default;
		DRW_Entities(DRW_Entities const&) = default;
		DRW_Entities(DRW_Block const& v) :DRW_Block(v) {}

		ptr_deque<DRW_Entity> entities; //stores the entities list

		template < typename Archive >
		friend Archive& operator << (Archive& ar, DRW_Entities const& v) {
			ar << (DRW_Block const&)v;

			ar.entityCount = {ar.FIRSTHANDLE};
			ar.WriteContainer<DRW_Entity>("entities"sv, v.entities.begin(), v.entities.end());

			return ar;
		}

	};

	/// @brief streams out ( json format )
	class oarchive_json {
	public:
		constexpr static inline int indent_width{2};
		using is_saving = std::true_type;
		using is_loading = std::false_type;

	public:
		std::ostream& ostream;
		std::ostringstream ss;
		int const nIndent;
		int version {};
		bool bAddComma_{};
		constexpr static int const FIRSTHANDLE{48};
		int entityCount{FIRSTHANDLE};

	public:
		oarchive_json(std::ostream& ostream) : ostream(ostream), nIndent{indent_width} {
			auto str = fmt::format("{0:>{1}}", '{', std::max(1, nIndent-indent_width+1));
			ss << str;
		}
		oarchive_json(oarchive_json& B) : ostream(B.ss), nIndent(B.nIndent+indent_width) {
			auto str = fmt::format("\n{0:>{1}}", '{', std::max(1, nIndent-indent_width+1));
			ss << str;
		}
		~oarchive_json() {
			auto str = fmt::format("\n{0:>{1}}", '}', std::max(1, nIndent-indent_width+1));
			ss << str;
			ostream << ss.str();
		}

		template < typename T >
		void Write(std::string_view key, T const& value) {
			if (bAddComma_) { ss << ","sv; } else { bAddComma_ = true; }

			auto str = fmt::format("\n{0:{1}}\"{2}\":", ' ', nIndent, key);
			ss << str;

			WriteObject<T>(value);

		}

		template < typename T >
		void WriteObject(T const& object) {

			using data_type = std::remove_cv_t<T>;
			std::array<char, 128> buf;
			auto PrintNumber = [&buf] < typename T >(T value) -> std::string {
				std::to_chars_result result;
				if constexpr (std::is_floating_point_v<T>) {
					 result = std::to_chars(buf.data(), buf.data()+buf.size(), value, std::chars_format::general);
				} else if constexpr (std::is_same_v<T, bool>) {
					return value ? "true"s : "false"s;
				} else {
					result = std::to_chars(buf.data(), buf.data()+buf.size(), value);
				}

				if (result.ec == std::errc {}) {
					return {buf.data(), result.ptr};
				} else {
					return {};
				}
			};

			if constexpr (std::is_same_v<data_type, std::string> or std::is_same_v<data_type, std::string_view>) {
				ss << "\"" << object << "\"";
			} else if constexpr (std::is_arithmetic_v<data_type>) {
				auto str = PrintNumber(object);
				ss << str;
			} else if constexpr (std::derived_from<data_type, DRW_Entity>) {
				oarchive_json ar2(*this);
				switch (object.eType) {
				case DRW::POINT:		ar2 << (DRW_Point&)(object); break;
				case DRW::LINE:			ar2 << (DRW_Line&)(object); break;
				case DRW::CIRCLE:		ar2 << (DRW_Circle&)(object); break;
				case DRW::ARC:			ar2 << (DRW_Arc&)(object); break;
				case DRW::SOLID:		ar2 << (DRW_Solid&)(object); break;
				case DRW::ELLIPSE:		ar2 << (DRW_Ellipse&)(object); break;
				case DRW::LWPOLYLINE:	ar2 << (DRW_LWPolyline&)(object); break;
				case DRW::POLYLINE:		ar2 << (DRW_Polyline&)(object); break;
				case DRW::SPLINE:		ar2 << (DRW_Spline&)(object); break;
				case DRW::INSERT:		ar2 << (DRW_Insert&)(object); break;
				case DRW::MTEXT:		ar2 << (DRW_MText&)(object); break;
				case DRW::TEXT:			ar2 << (DRW_Text&)(object); break;
				case DRW::DIMLINEAR:
				case DRW::DIMALIGNED:
				case DRW::DIMANGULAR:
				case DRW::DIMANGULAR3P:
				case DRW::DIMRADIAL:
				case DRW::DIMDIAMETRIC:
				case DRW::DIMORDINATE:	ar2 << (DRW_Dimension&)(object); break;
				case DRW::LEADER:		ar2 << (DRW_Leader&)(object); break;
				case DRW::HATCH:		ar2 << (DRW_Hatch&)(object); break;
				case DRW::IMAGE:		ar2 << (DRW_ImagePath&)(object); break;
				case DRW::BLOCK:		ar2 << (DRW_Entities&)(object); break;
				case DRW::UNKNOWN :
					break;
				}
			} else if constexpr (std::is_same_v<data_type, DRW_Coord>) {
				DRW_Coord const& coord = object;
				ss << "["sv;
				ss << PrintNumber(coord.x) << ","sv;
				ss << PrintNumber(coord.y) << ","sv;
				ss << PrintNumber(coord.z) << "]"sv;
			} else if constexpr (std::is_same_v<data_type, DRW_Vertex2D>) {
				DRW_Vertex2D const& vertex = object;
				ss << "["sv;
				ss << PrintNumber(vertex.x) << ","sv;
				ss << PrintNumber(vertex.y) << ","sv;
				ss << PrintNumber(vertex.stawidth) << ","sv;
				ss << PrintNumber(vertex.endwidth) << ","sv;
				ss << PrintNumber(vertex.bulge) << "]"sv;
			} else if constexpr (std::is_same_v<data_type, DRW_Variant>) {
				DRW_Variant const& var = object;

				//ss << "{"sv;
				switch (var.type()) {
				//case DRW_Variant::STRING:   ss << "\"string\":"sv << *var.content.s; break;
				//case DRW_Variant::INTEGER:  ss << "\"integer\":"sv << var.content.i; break;
				//case DRW_Variant::DOUBLE:   ss << "\"double\":"sv << var.content.d; break;
				//case DRW_Variant::COORD:    ss << "\"coord\":"sv; WriteObject(*var.content.v); break;
				case DRW_Variant::STRING:   WriteObject(*var.content.s); break;
				case DRW_Variant::INTEGER:  WriteObject(var.content.i); break;
				case DRW_Variant::DOUBLE:   WriteObject(var.content.d); break;
				case DRW_Variant::COORD:    WriteObject(*var.content.v); break;
				}
				//ss << "}"sv;

			} else if constexpr (std::is_same_v<data_type, std::pair<const std::string, DRW_Variant const*> >) {
				std::pair<const std::string, DRW_Variant const*> const& pair = object;
				auto str = fmt::format("\n{0:{1}}\"{2}\":", ' ', nIndent+indent_width, pair.first);
				ss << str;

				WriteObject<DRW_Variant>(*pair.second);

			} else {
				oarchive_json ar2(*this);
				ar2 << object;
			}
		}
		template < typename T >
		void WriteObject(std::unique_ptr<T> const& ref) {
			WriteObject<T>(*ref);
		}
		template < typename T >
		void WriteObject(std::shared_ptr<T> const& ref) {
			WriteObject<T>(*ref);
		}

		template < typename T >
		void WriteObject(T const* ptr) {
			WriteObject<T>(*ptr);
		}

		template < typename T >
		void WriteContainer(std::string_view key, std::span<T> const arr) {
			WriteContainer<T>(key, arr.begin(), arr.end());
		}
		template < typename T >
		void WriteContainer(std::string_view key, std::vector<T> const& arr) {
			WriteContainer<T>(key, arr.begin(), arr.end());
		}
		template < typename T >
		void WriteContainer(std::string_view key, std::deque<T> const& arr) {
			WriteContainer<T>(key, arr.begin(), arr.end());
		}

		template < typename T, typename T_ITER >
		void WriteContainer(std::string_view key, T_ITER begin, T_ITER end) {
			if (bAddComma_) { ss << ","; } else { bAddComma_ = true; }

			auto str = fmt::format("\n{0:{1}}\"{2}\":", ' ', nIndent, key);
			ss << str;

			ss << "[ ";
			bool bAddComma{};
			int count{};
			for (auto iter = begin; iter != end; iter++, count++) {
				auto const& v = *iter;
				
				if (bAddComma) { ss << ","; } else { bAddComma = true; }
				if ((count+1)%4 == 0) {
					auto str = fmt::format("\n{0:{1}}", ' ', nIndent);
					ss << str;
				}

				WriteObject<T>(v);

			}

			ss << "] ";
		}

	};

	//-----------------------------------------------------------------------------
	// DXFLoaderDefault
	class CDXFConverter : private DRW_Interface {
	public:
		DRW_Header header;
		std::deque<DRW_LType> lineTypes;
		std::deque<DRW_Layer> layers;
		std::deque<DRW_Dimstyle> dimStyles;
		std::deque<DRW_Vport> viewports;
		std::deque<DRW_Textstyle> textStyles;
		std::deque<DRW_AppId> appIds;
		std::deque<DRW_Entities> blocks;    //stores a copy of all blocks and the entities in it
		std::deque<DRW_ImagePath> images;      //temporary list to find images for link with DRW_ImageDef. Do not delete it!!

		DRW_Entities mainBlock;
		DRW_Entities* currentBlock {};

	public:

		bool fileImport(std::filesystem::path fileI) {
			auto ext = MakeLower<wchar_t>(fileI.extension().wstring());
			currentBlock = &mainBlock;

			if (ext == L".dxf") {
				//loads dxf
				auto dxf = std::make_unique<dxfRW>(fileI);
				bool success = dxf->read(this, false);

				return success;
			} else if (ext == L".dwg") {
				//loads dwg
				auto dwg = std::make_unique<dwgR>(fileI);
				bool success = dwg->read(this, false);

				return success;
			}

			return false;
		}


		template < typename Archive >
		bool Export(Archive& ar) {
			using namespace std::literals;

			ar.Write("header"sv, header);

			ar.WriteContainer<DRW_LType>("lineTypes"sv, lineTypes);
			ar.WriteContainer<DRW_Layer>("layers"sv, layers);
			ar.WriteContainer<DRW_Dimstyle>("dimStyle"sv, dimStyles);
			ar.WriteContainer<DRW_Vport>("viewports"sv, viewports);
			ar.WriteContainer<DRW_Textstyle>("textStyles"sv, textStyles);
			ar.WriteContainer<DRW_AppId>("appIds"sv, appIds);
			ar.WriteContainer<DRW_Entities>("blocks"sv, blocks);
			ar.WriteContainer<DRW_ImagePath>("images"sv, images);
			ar.Write("mainBlock"sv, mainBlock);

			return true;
		}

		//reimplement virtual DRW_Interface functions

		//reader part, stores all in class dx_data
		//header
		void addHeader(const DRW_Header* data) override {
			header = *data;
		}

		//tables
		virtual void addLType(const DRW_LType& data) override {
			lineTypes.push_back(data);
		}
		virtual void addLayer(const DRW_Layer& data) override {
			layers.push_back(data);
		}
		virtual void addDimStyle(const DRW_Dimstyle& data) override {
			dimStyles.push_back(data);
		}
		virtual void addVport(const DRW_Vport& data) override {
			viewports.push_back(data);
		}
		virtual void addTextStyle(const DRW_Textstyle& data) override {
			textStyles.push_back(data);
		}
		virtual void addAppId(const DRW_AppId& data) override {
			appIds.push_back(data);
		}

		//blocks
		virtual void addBlock(const DRW_Block& data) override {
			blocks.emplace_back(data);
			currentBlock = &blocks.back();
		}
		virtual void endBlock() {
			currentBlock = &mainBlock;
		}

		virtual void setBlock(const int /*handle*/) {}//unused

													 //entities
		virtual void addPoint(const DRW_Point& data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Point>(data));
		}
		virtual void addLine(const DRW_Line& data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Line>(data));
		}
		virtual void addRay(const DRW_Ray& data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Ray>(data));
		}
		virtual void addXline(const DRW_Xline& data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Xline>(data));
		}
		virtual void addArc(const DRW_Arc& data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Arc>(data));
		}
		virtual void addCircle(const DRW_Circle& data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Circle>(data));
		}
		virtual void addEllipse(const DRW_Ellipse& data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Ellipse>(data));
		}
		virtual void addLWPolyline(const DRW_LWPolyline& data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_LWPolyline>(data));
		}
		virtual void addPolyline(const DRW_Polyline& data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Polyline>(data));
		}
		virtual void addSpline(const DRW_Spline* data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Spline>(*data));
		}
		virtual void addKnot(const DRW_Entity& data) override {};
		virtual void addInsert(const DRW_Insert& data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Insert>(data));
		}
		virtual void addTrace(const DRW_Trace& data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Trace>(data));
		}
		virtual void add3dFace(const DRW_3Dface& data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_3Dface>(data));
		}
		virtual void addSolid(const DRW_Solid& data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Solid>(data));
		}
		virtual void addMText(const DRW_MText& data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_MText>(data));
		}
		virtual void addText(const DRW_Text& data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Text>(data));
		}
		virtual void addDimAlign(const DRW_DimAligned* data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_DimAligned>(*data));
		}
		virtual void addDimLinear(const DRW_DimLinear* data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_DimLinear>(*data));
		}
		virtual void addDimRadial(const DRW_DimRadial* data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_DimRadial>(*data));
		}
		virtual void addDimDiametric(const DRW_DimDiametric* data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_DimDiametric>(*data));
		}
		virtual void addDimAngular(const DRW_DimAngular* data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_DimAngular>(*data));
		}
		virtual void addDimAngular3P(const DRW_DimAngular3p* data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_DimAngular3p>(*data));
		}
		virtual void addDimOrdinate(const DRW_DimOrdinate* data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_DimOrdinate>(*data));
		}
		virtual void addLeader(const DRW_Leader* data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Leader>(*data));
		}
		virtual void addHatch(const DRW_Hatch* data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Hatch>(*data));
		}
		virtual void addViewport(const DRW_Viewport& data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Viewport>(data));
		}
		virtual void addImage(const DRW_Image* data) override {
			currentBlock->entities.push_back(std::make_unique<DRW_Image>(*data));
			images.emplace_back(*data);
		}

		virtual void linkImage(const DRW_ImageDef* data) override {
			duint32 handle = data->handle;
			std::string path(data->name);
			for (auto& image : images) {
				if (image.ref == handle) {
					image.path = path;
				}
			}
		}

		//writer part, send all in class dx_data to writer
		virtual void addComment(const char* /*comment*/) {}

		virtual void writeHeader(DRW_Header& data) override {};
		virtual void writeBlocks() override {};
		virtual void writeBlockRecords() override {};
		virtual void writeEntities() override {};
		virtual void writeLTypes() override {};
		virtual void writeLayers() override {};
		virtual void writeTextstyles() override {};
		virtual void writeVports() override {};
		virtual void writeDimstyles() override {};
		virtual void writeAppId() override {};

	};


}
