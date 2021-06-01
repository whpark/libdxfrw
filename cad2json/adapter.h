#pragma once

#include <array>
#include <string>
#include <string_view>

#include "drw_base.h"
#include "drw_entities.h"
#include "drw_objects.h"

#define WRITE_MEMBER(member)        Write(#member, v.member)
#define WRITE_MEMBER_ENUM(member)   Write(#member, (int)v.member)
#define WRITE_MEMBER_ANGLE(member)  Write(#member, v.member *ARAD)

using namespace std::literals;


template < typename tchar_t >
inline std::basic_string<tchar_t> MakeUpper(std::basic_string_view<tchar_t> sv) {
	std::basic_string<tchar_t> str(sv.size(), {});
	std::transform(sv.begin(), sv.end(), str.begin(), ::toupper);
	return str;
};
template < typename tchar_t >
inline std::basic_string<tchar_t> MakeLower(std::basic_string_view<tchar_t> sv) {
	std::basic_string<tchar_t> str(sv.size(), {});
	std::transform(sv.begin(), sv.end(), str.begin(), ::tolower);
	return str;
};


template < typename tchar_t >
inline std::basic_string_view<tchar_t> TrimLeft(std::basic_string_view<tchar_t> sv) {
	auto pos = sv.find_first_not_of(' ');
	if (pos == sv.npos)
		return {};
	return {sv.data()+pos, sv.size()-pos};
};


//template < typename Archive >
//Archive& operator << (Archive& ar, DRW_Coord const& v) {
//
//	ar.WRITE_MEMBER(x);
//	ar.WRITE_MEMBER(y);
//	ar.WRITE_MEMBER(z);
//
//	return ar;
//}

//template < typename Archive >
//Archive& operator << (Archive& ar, DRW_Vertex2D const& v) {
//
//	ar.WRITE_MEMBER(x);
//	ar.WRITE_MEMBER(y);
//	ar.WRITE_MEMBER(stawidth);
//	ar.WRITE_MEMBER(endwidth);
//	ar.WRITE_MEMBER(bulge);
//
//	return ar;
//}

//template < typename Archive >
//Archive& operator << (Archive& ar, DRW_Variant const& v) {
//
//	switch (v.type()) {
//	case DRW_Variant::STRING:   ar.Write("string"sv, *v.content.s); break;
//	case DRW_Variant::INTEGER:  ar.Write("integer"sv, v.content.i); break;
//	case DRW_Variant::DOUBLE:   ar.Write("double"sv, v.content.d); break;
//	case DRW_Variant::COORD:    ar.Write("coord"sv, *v.content.v); break;
//	}
//
//	return ar;
//}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Entity const& v) {

	static std::array<std::string, DRW::UNKNOWN> const strEntityNames = { {
		"3dFace"s,
		"ARC"s,
		"BLOCK"s,
		"CIRCLE"s,
		"DIMENSION"s,
		"DIMALIGNED"s,
		"DIMLINEAR"s,
		"DIMRADIAL"s,
		"DIMDIAMETRIC"s,
		"DIMANGULAR"s,
		"DIMANGULAR3P"s,
		"DIMORDINATE"s,
		"ELLIPSE"s,
		"HATCH"s,
		"IMAGE"s,
		"INSERT"s,
		"LEADER"s,
		"LINE"s,
		"LWPOLYLINE"s,
		"MTEXT"s,
		"POINT"s,
		"POLYLINE"s,
		"RAY"s,
		"SOLID"s,
		"SPLINE"s,
		"TEXT"s,
		"TRACE"s,
		"UNDERLAY"s,
		"VERTEX"s,
		"VIEWPORT"s,
		"XLINE"s,
	}};


	ar.WRITE_MEMBER_ENUM(eType);

	if (v.eType >= 0 and v.eType < strEntityNames.size())
		ar.Write("entityName", strEntityNames[v.eType]);

	//handle = ++ar.entityCount;
	//ar.WRITE_MEMBER(handle);

	//ar.WRITE_MEMBER(parentHandle);
	//std::list<std::list<DRW_Variant> > appData;

	ar.WRITE_MEMBER_ENUM(space);

	// Layer
	auto svLayer { TrimLeft<char>(v.layer)};
	ar.Write("layer", svLayer);

	ar.WRITE_MEMBER(lineType);
	ar.WRITE_MEMBER(material);
	ar.WRITE_MEMBER(color);
	if (v.color24 >= 0)
		ar.WRITE_MEMBER(color24);
	ar.WRITE_MEMBER_ENUM(lWeight);

	//ar.WRITE_MEMBER(ltypeScale);
	//ar.WRITE_MEMBER(visible);
	//ar.WRITE_MEMBER(numProxyGraph);
	//ar.WRITE_MEMBER(proxyGraphics);
	//ar.WRITE_MEMBER(colorName);
	//ar.WRITE_MEMBER(transparency);
	//ar.WRITE_MEMBER(plotStyle);
	//ar.WRITE_MEMBER_ENUM(shadow);
	
	// todo : check
	//ar.WRITE_MEMBER(haveExtrusion);

	if (v.extData.size())
		ar.WriteContainer<DRW_Variant>("extData"sv, v.extData.begin(), v.extData.end());

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Point const& v) {
	ar << (DRW_Entity const&)v;

	ar.WRITE_MEMBER(basePoint);
	//ar.WRITE_MEMBER(thickness);
	//ar.WRITE_MEMBER(extPoint);

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Line const& v) {
	ar << (DRW_Point const&)v;

	ar.WRITE_MEMBER(secPoint);

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Ray const& v) {

	v.secPoint.unitize();

	ar << (DRW_Line const&)v;

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Xline const& v) {
	ar << (DRW_Ray const&)v;
	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Circle const& v) {
	ar << (DRW_Point const&)v;

	ar.WRITE_MEMBER(radious);

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Arc const& v) {
	ar << (DRW_Circle const&)v;

	ar.WRITE_MEMBER_ANGLE(staangle);
	ar.WRITE_MEMBER_ANGLE(endangle);
	ar.WRITE_MEMBER(isccw);

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Ellipse const& v) {
	ar << (DRW_Line const&)v;

	ar.WRITE_MEMBER(ratio);
	ar.WRITE_MEMBER_ANGLE(staparam);
	ar.WRITE_MEMBER_ANGLE(endparam);
	ar.WRITE_MEMBER(isccw);

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Trace const& v) {
	ar << (DRW_Line const&)v;

	ar.WRITE_MEMBER(thirdPoint);
	ar.WRITE_MEMBER(fourPoint);

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Solid const& v) {
	ar << (DRW_Trace const&)v;
	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_3Dface const& v) {
	ar << (DRW_Trace const&)v;

	ar.WRITE_MEMBER(invisibleflag);

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Block const& v) {
	ar << (DRW_Point const&)v;

	ar.WRITE_MEMBER(name);
	ar.WRITE_MEMBER(flags);

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Insert const& v) {
	ar << (DRW_Point const&)v;

	// todo : Check

	ar.WRITE_MEMBER(name);
	ar.WRITE_MEMBER(xscale);
	ar.WRITE_MEMBER(yscale);
	ar.WRITE_MEMBER(zscale);
	ar.WRITE_MEMBER_ANGLE(angle);
	ar.WRITE_MEMBER(colcount);
	ar.WRITE_MEMBER(rowcount);
	ar.WRITE_MEMBER(colspace);
	ar.WRITE_MEMBER(rowspace);

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_LWPolyline const& v) {
	ar << (DRW_Entity const&)v;

	//ar.Write("vertexnum", vertlist.size());
	ar.WRITE_MEMBER(flags);
	ar.WRITE_MEMBER(width);
	ar.WRITE_MEMBER(elevation);
	ar.WRITE_MEMBER(thickness);

	ar.WriteContainer<DRW_Vertex2D>("vertlist"sv, v.vertlist.begin(), v.vertlist.end());

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Text const& v) {
	ar << (DRW_Line const&)v;

	ar.WRITE_MEMBER(height);
	ar.WRITE_MEMBER(text);
	ar.WRITE_MEMBER(angle);
	ar.WRITE_MEMBER(widthscale);
	ar.WRITE_MEMBER(oblique);
	ar.WRITE_MEMBER(style);
	ar.WRITE_MEMBER(textgen);
	ar.WRITE_MEMBER_ENUM(alignH);
	ar.WRITE_MEMBER_ENUM(alignV);
	//ar.WRITE_MEMBER(styleH;

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_MText const& v) {
	ar << (DRW_Text const&)v;

	ar.WRITE_MEMBER(interlin);

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Vertex const* v) {
	return ar << *v;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Vertex const& v) {
	//ar << (DRW_Point const&)v;
	if ( (v.flags & 128) && !(v.flags & 64) ) {
	} else {
		ar.WRITE_MEMBER(basePoint);
	}

	ar.WRITE_MEMBER(stawidth);
	ar.WRITE_MEMBER(endwidth);
	ar.WRITE_MEMBER(bulge);
	ar.WRITE_MEMBER(flags);
	ar.WRITE_MEMBER(tgdir);
	if (v.flags & 128) {
		ar.WRITE_MEMBER(vindex1);
		ar.WRITE_MEMBER(vindex2);
		ar.WRITE_MEMBER(vindex3);
		ar.WRITE_MEMBER(vindex4);
		ar.WRITE_MEMBER(identifier);
	}

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Polyline const& v) {
	//ar << (DRW_Point const&)v;
	ar << (DRW_Entity const&)v;

	DRW_Coord base {0,0,v.basePoint.z};
	ar.Write("basePoint", base);
	ar.WRITE_MEMBER(thickness);
	ar.WRITE_MEMBER(flags);
	ar.WRITE_MEMBER(defstawidth);
	ar.WRITE_MEMBER(defendwidth);
	ar.WRITE_MEMBER(vertexcount);
	ar.WRITE_MEMBER(facecount);
	ar.WRITE_MEMBER(smoothM);
	ar.WRITE_MEMBER(smoothN);
	ar.WRITE_MEMBER(curvetype);

	ar.WriteContainer<DRW_Vertex>("vertlist"sv, v.vertlist.begin(), v.vertlist.end());

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Spline const& v) {
	ar << (DRW_Entity const&)v;

	ar.WRITE_MEMBER(normalVec);
	ar.WRITE_MEMBER(flags);
	ar.WRITE_MEMBER(degree);
	//ar.WRITE_MEMBER(nknots);
	//ar.WRITE_MEMBER(ncontrol);
	//ar.WRITE_MEMBER(nfit);
	ar.WRITE_MEMBER(tolknot);
	ar.WRITE_MEMBER(tolcontrol);
	ar.WRITE_MEMBER(tolfit);
	//ar.WRITE_MEMBER(tgStart);
	//ar.WRITE_MEMBER(tgEnd);

	using double_t = double;	// why?????
	ar.WriteContainer<double_t>("knotslist"sv, v.knotslist.begin(), v.knotslist.end());
	ar.WriteContainer<DRW_Coord>("controllist"sv, v.controllist.begin(), v.controllist.end());
	ar.WriteContainer<DRW_Coord>("fitlist"sv, v.fitlist.begin(), v.fitlist.end());

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_HatchLoop const& v) {

	ar.WRITE_MEMBER(type);
	if (v.type & 2) {
	} else {
		ar.WriteContainer<DRW_Entity>("objlist"sv, v.objlist.begin(), v.objlist.end());
	}

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Hatch const& v) {
	ar << (DRW_Point const&)v;
	ar.WRITE_MEMBER(extPoint);

	ar.WRITE_MEMBER(name);
	ar.WRITE_MEMBER(solid);
	ar.WRITE_MEMBER(associative);

	ar.WRITE_MEMBER(hstyle);
	ar.WRITE_MEMBER(hpattern);
	ar.WRITE_MEMBER(doubleflag);
	ar.WRITE_MEMBER(loopsnum);
	ar.WRITE_MEMBER(angle);
	ar.WRITE_MEMBER(scale);
	ar.WRITE_MEMBER(deflines);

	ar.WriteContainer<DRW_HatchLoop>("looplist"sv, v.looplist.begin(), v.looplist.end());

	return ar;
}
template < typename Archive >
Archive& operator << (Archive& ar, DRW_Hatch const* v) {
	return ar << *v;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Image const& v) {
	ar << (DRW_Line const&)v;

	//ar.WRITE_MEMBER(ref);
	ar.WRITE_MEMBER(vVector);
	ar.WRITE_MEMBER(sizeu);
	ar.WRITE_MEMBER(sizev);
	//ar.WRITE_MEMBER(dz);
	ar.WRITE_MEMBER(clip);
	ar.WRITE_MEMBER(brightness);
	ar.WRITE_MEMBER(contrast);
	ar.WRITE_MEMBER(fade);

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Dimension const& v) {
	ar << (DRW_Entity const&)v;

	ar.WRITE_MEMBER(type);

	ar.Write("defPoint", v.getDefPoint());
	ar.Write("textPoint", v.getTextPoint());
	auto type = v.type|32;
	ar.Write("type", type);
	ar.Write("text", v.getText());
	ar.Write("align", v.getAlign());
	ar.Write("textLineStyle", v.getTextLineStyle());
	ar.Write("textLineFactor", v.getTextLineFactor());
	ar.Write("style", v.getStyle());
	ar.Write("dir", v.getDir());
	ar.Write("extrution", v.getExtrusion());

    switch (v.eType) {
    case DRW::DIMALIGNED:
    case DRW::DIMLINEAR:
		{
			auto* dd = (DRW_DimAligned const*)&v;
			DRW_Coord crd = dd->getClonepoint();
			ar.Write("Clonepoint", crd);
			ar.Write("Def1Point", dd->getDef1Point());
			ar.Write("Def2Point", dd->getDef2Point());

			if (dd->eType == DRW::DIMLINEAR) {
				auto* dl = (DRW_DimLinear const*)&v;
				ar.Write("angle", dl->getAngle());
				ar.Write("oblique", dl->getOblique());
			}
		}
        break;

    case DRW::DIMRADIAL:
		{
			auto* dd = (DRW_DimRadial const*)&v;
			ar.Write("diameterPoint", dd->getDiameterPoint());
			ar.Write("leaderLength", dd->getLeaderLength());
		}
        break;

    case DRW::DIMDIAMETRIC:
		{
			auto* dd = (DRW_DimDiametric const*)&v;
			ar.Write("diameter1Point", dd->getDiameter1Point());
			ar.Write("leaderLength", dd->getLeaderLength());
		}
        break;

    case DRW::DIMANGULAR:
		{
			auto* dd = (DRW_DimAngular const*)&v;
			ar.Write("firstLine1", dd->getFirstLine1());
			ar.Write("firstLine2", dd->getFirstLine2());
			ar.Write("secondLine1", dd->getSecondLine1());
			ar.Write("dimPoint", dd->getDimPoint());
		}
        break;

    case DRW::DIMANGULAR3P:
		{
			auto* dd = (DRW_DimAngular3p const*)&v;
			ar.Write("firstLine", dd->getFirstLine());
			ar.Write("secondLine", dd->getSecondLine());
			ar.Write("vertexPoint", dd->getVertexPoint());
		}
        break;

    case DRW::DIMORDINATE:
		{
	        auto* dd = (DRW_DimOrdinate const*)&v;
			ar.Write("firstLine", dd->getFirstLine());
			ar.Write("secondLine", dd->getSecondLine());
        }
		break;

    default:
        break;
    }
	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Leader const& v) {
	ar << (DRW_Entity const&)v;

	ar.WRITE_MEMBER(style);
	ar.WRITE_MEMBER(arrow);
	ar.WRITE_MEMBER(leadertype);
	ar.WRITE_MEMBER(flag);
	ar.WRITE_MEMBER(hookline);
	ar.WRITE_MEMBER(hookflag);
	ar.WRITE_MEMBER(textheight);
	ar.WRITE_MEMBER(textwidth);
	//ar.WRITE_MEMBER(vertnum);
	ar.WriteContainer<DRW_Coord>("vertexlist"sv, v.vertexlist.begin(), v.vertexlist.end());
	//ar.WRITE_MEMBER(coloruse);
	//ar.WRITE_MEMBER(annotHandle);
	//ar.WRITE_MEMBER(extrusionPoint);
	//ar.WRITE_MEMBER(horizdir);
	//ar.WRITE_MEMBER(offsetblock);
	//ar.WRITE_MEMBER(offsettext);

	return ar;
}


template < typename Archive >
Archive& operator << (Archive& ar, DRW_Viewport const& v) {
	ar << (DRW_Point const&)v;

	ar.WRITE_MEMBER(pswidth);
	ar.WRITE_MEMBER(psheight);
	ar.WRITE_MEMBER(vpstatus);
	ar.WRITE_MEMBER(vpID);
	ar.WRITE_MEMBER(centerPX);
	ar.WRITE_MEMBER(centerPY);
	//ar.WRITE_MEMBER(snapPX);
	//ar.WRITE_MEMBER(snapPY);
	//ar.WRITE_MEMBER(snapSpPX);
	//ar.WRITE_MEMBER(snapSpPY);
	//ar.WRITE_MEMBER(viewDir);
	//ar.WRITE_MEMBER(viewTarget);
	//ar.WRITE_MEMBER(viewLength);
	//ar.WRITE_MEMBER(frontClip);
	//ar.WRITE_MEMBER(backClip);
	//ar.WRITE_MEMBER(viewHeight);
	//ar.WRITE_MEMBER(snapAngle);
	//ar.WRITE_MEMBER(twistAngle);

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Header const& v) {
	//ar.WriteContainer<std::pair<const std::string, DRW_Variant const*>>("var"sv, v.vars.begin(), v.vars.end());
	ar.WRITE_MEMBER(vars);

	return ar;
}

//template < typename Archive >
//Archive& operator << (Archive& ar, std::pair<const std::string, DRW_Variant const*> const& v) {
//	if (v.second->type() != DRW_Variant::INVALID)
//		ar.Write(v.first, *v.second);
//	return ar;
//}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_TableEntry const& v) {
	ar.WRITE_MEMBER_ENUM(tType);

	static std::array<std::string, DRW::TTYPE::IMAGEDEF+1> const strNames = {{
		"UNKNOWNT"s,
		"LTYPE"s,
		"LAYER"s,
		"STYLE"s,
		"DIMSTYLE"s,
		"VPORT"s,
		"BLOCK_RECORD"s,
		"APPID"s,
		"IMAGEDEF"s
	}};
	if (v.tType >= 0 and v.tType < strNames.size())
		ar.Write("TypeName"sv, strNames[v.tType]);

	ar.WRITE_MEMBER(name);
	ar.WRITE_MEMBER(flags);
	//if (v.extData.size())
	//	ar.WriteContainer<DRW_Variant>("extData"sv, v.extData.begin(), v.extData.end());

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Dimstyle const& v) {
	ar << (DRW_TableEntry const&)v;

	ar.WRITE_MEMBER(dimpost);
	ar.WRITE_MEMBER(dimapost);
	ar.WRITE_MEMBER(dimblk);
	ar.WRITE_MEMBER(dimblk1);
	ar.WRITE_MEMBER(dimblk2);
	ar.WRITE_MEMBER(dimscale);
	ar.WRITE_MEMBER(dimasz);
	ar.WRITE_MEMBER(dimexo);
	ar.WRITE_MEMBER(dimdli);
	ar.WRITE_MEMBER(dimexe);
	ar.WRITE_MEMBER(dimrnd);
	ar.WRITE_MEMBER(dimdle);
	ar.WRITE_MEMBER(dimtp);
	ar.WRITE_MEMBER(dimtm);
	ar.WRITE_MEMBER(dimfxl);
	ar.WRITE_MEMBER(dimtxt);
	ar.WRITE_MEMBER(dimcen);
	ar.WRITE_MEMBER(dimtsz);
	ar.WRITE_MEMBER(dimaltf);
	ar.WRITE_MEMBER(dimlfac);
	ar.WRITE_MEMBER(dimtvp);
	ar.WRITE_MEMBER(dimtfac);
	ar.WRITE_MEMBER(dimgap);
	ar.WRITE_MEMBER(dimaltrnd);
	ar.WRITE_MEMBER(dimtol);
	ar.WRITE_MEMBER(dimlim);
	ar.WRITE_MEMBER(dimtih);
	ar.WRITE_MEMBER(dimtoh);
	ar.WRITE_MEMBER(dimse1);
	ar.WRITE_MEMBER(dimse2);
	ar.WRITE_MEMBER(dimtad);
	ar.WRITE_MEMBER(dimzin);
	ar.WRITE_MEMBER(dimazin);
	ar.WRITE_MEMBER(dimalt);
	ar.WRITE_MEMBER(dimaltd);
	ar.WRITE_MEMBER(dimtofl);
	ar.WRITE_MEMBER(dimsah);
	ar.WRITE_MEMBER(dimtix);
	ar.WRITE_MEMBER(dimsoxd);
	ar.WRITE_MEMBER(dimclrd);
	ar.WRITE_MEMBER(dimclre);
	ar.WRITE_MEMBER(dimclrt);
	ar.WRITE_MEMBER(dimadec);
	ar.WRITE_MEMBER(dimunit);
	ar.WRITE_MEMBER(dimdec);
	ar.WRITE_MEMBER(dimtdec);
	ar.WRITE_MEMBER(dimaltu);
	ar.WRITE_MEMBER(dimalttd);
	ar.WRITE_MEMBER(dimaunit);
	ar.WRITE_MEMBER(dimfrac);
	ar.WRITE_MEMBER(dimlunit);
	ar.WRITE_MEMBER(dimdsep);
	ar.WRITE_MEMBER(dimtmove);
	ar.WRITE_MEMBER(dimjust);
	ar.WRITE_MEMBER(dimsd1);
	ar.WRITE_MEMBER(dimsd2);
	ar.WRITE_MEMBER(dimtolj);
	ar.WRITE_MEMBER(dimtzin);
	ar.WRITE_MEMBER(dimaltz);
	ar.WRITE_MEMBER(dimaltttz);
	ar.WRITE_MEMBER(dimfit);
	ar.WRITE_MEMBER(dimupt);
	ar.WRITE_MEMBER(dimatfit);
	ar.WRITE_MEMBER(dimfxlon);
	ar.WRITE_MEMBER(dimtxsty);
	ar.WRITE_MEMBER(dimldrblk);
	ar.WRITE_MEMBER(dimlwd);
	ar.WRITE_MEMBER(dimlwe);

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_LType const& v) {
	ar << (DRW_TableEntry const&)v;

	std::string strLineType = MakeUpper<char>(v.name);
	if (strLineType == "BYLAYER" || strLineType == "BYBLOCK" || strLineType == "CONTINUOUS") {
		return ar;
	}

	//ar.WRITE_MEMBER(name);
	//ar.WRITE_MEMBER(flags);
	ar.WRITE_MEMBER(desc);
	ar.WriteContainer<double_t>("path"sv, v.path.begin(), v.path.end());

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Layer const& v) {
	ar << (DRW_TableEntry const&)v;

	ar.WRITE_MEMBER(color);
	if (v.color24 >= 0)
		ar.WRITE_MEMBER(color24);
	ar.WRITE_MEMBER(plotF);
	ar.WRITE_MEMBER_ENUM(lWeight);
	ar.WRITE_MEMBER(lineType);

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Textstyle const& v) {
	ar << (DRW_TableEntry const&)v;

	ar.WRITE_MEMBER(height);
	ar.WRITE_MEMBER(width);
	ar.WRITE_MEMBER(oblique);
	ar.WRITE_MEMBER(genFlag);
	ar.WRITE_MEMBER(lastHeight);
	ar.WRITE_MEMBER(font);
	ar.WRITE_MEMBER(bigFont);
	ar.WRITE_MEMBER(fontFamily);

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_Vport const& v) {
	ar << (DRW_TableEntry const&)v;

	ar.WRITE_MEMBER(lowerLeft);
	ar.WRITE_MEMBER(UpperRight);
	ar.WRITE_MEMBER(center);
	ar.WRITE_MEMBER(snapBase);
	ar.WRITE_MEMBER(snapSpacing);
	ar.WRITE_MEMBER(gridSpacing);
	ar.WRITE_MEMBER(viewDir);
	ar.WRITE_MEMBER(viewTarget);
	ar.WRITE_MEMBER(height);
	ar.WRITE_MEMBER(ratio);
	ar.WRITE_MEMBER(lensHeight);
	ar.WRITE_MEMBER(frontClip);
	ar.WRITE_MEMBER(backClip);
	ar.WRITE_MEMBER(snapAngle);
	ar.WRITE_MEMBER(twistAngle);
	ar.WRITE_MEMBER(viewMode);
	ar.WRITE_MEMBER(circleZoom);
	ar.WRITE_MEMBER(fastZoom);
	ar.WRITE_MEMBER(ucsIcon);
	ar.WRITE_MEMBER(snap);
	ar.WRITE_MEMBER(grid);
	ar.WRITE_MEMBER(snapStyle);
	ar.WRITE_MEMBER(snapIsopair);
	ar.WRITE_MEMBER(gridBehavior);

	return ar;
}

template < typename Archive >
Archive& operator << (Archive& ar, DRW_AppId const& v) {
	ar << (DRW_TableEntry const&)v;

	return ar;
}

