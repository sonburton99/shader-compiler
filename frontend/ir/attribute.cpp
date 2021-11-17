// Copyright 2021 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <fmt/format.h>

#include "shader_recompiler/exception.h"
#include "shader_recompiler/frontend/ir/attribute.h"

namespace Shader::IR {

bool IsGeneric(Attribute attribute) noexcept {
    return attribute >= Attribute::Generic0X && attribute <= Attribute::Generic31X;
}

u32 GenericAttributeIndex(Attribute attribute) {
    if (!IsGeneric(attribute)) {
        throw InvalidArgument("Attribute is not generic {}", attribute);
    }
    return (static_cast<u32>(attribute) - static_cast<u32>(Attribute::Generic0X)) / 4u;
}

u32 GenericAttributeElement(Attribute attribute) {
    if (!IsGeneric(attribute)) {
        throw InvalidArgument("Attribute is not generic {}", attribute);
    }
    return static_cast<u32>(attribute) % 4;
}

std::string NameOf(Attribute attribute) {
    switch (attribute) {
    case Attribute::PrimitiveId:
        return "PrimitiveId";
    case Attribute::Layer:
        return "Layer";
    case Attribute::ViewportIndex:
        return "ViewportIndex";
    case Attribute::PointSize:
        return "PointSize";
    case Attribute::PositionX:
        return "Position.X";
    case Attribute::PositionY:
        return "Position.Y";
    case Attribute::PositionZ:
        return "Position.Z";
    case Attribute::PositionW:
        return "Position.W";
    case Attribute::Generic0X:
        return "Generic[0].X";
    case Attribute::Generic0Y:
        return "Generic[0].Y";
    case Attribute::Generic0Z:
        return "Generic[0].Z";
    case Attribute::Generic0W:
        return "Generic[0].W";
    case Attribute::Generic1X:
        return "Generic[1].X";
    case Attribute::Generic1Y:
        return "Generic[1].Y";
    case Attribute::Generic1Z:
        return "Generic[1].Z";
    case Attribute::Generic1W:
        return "Generic[1].W";
    case Attribute::Generic2X:
        return "Generic[2].X";
    case Attribute::Generic2Y:
        return "Generic[2].Y";
    case Attribute::Generic2Z:
        return "Generic[2].Z";
    case Attribute::Generic2W:
        return "Generic[2].W";
    case Attribute::Generic3X:
        return "Generic[3].X";
    case Attribute::Generic3Y:
        return "Generic[3].Y";
    case Attribute::Generic3Z:
        return "Generic[3].Z";
    case Attribute::Generic3W:
        return "Generic[3].W";
    case Attribute::Generic4X:
        return "Generic[4].X";
    case Attribute::Generic4Y:
        return "Generic[4].Y";
    case Attribute::Generic4Z:
        return "Generic[4].Z";
    case Attribute::Generic4W:
        return "Generic[4].W";
    case Attribute::Generic5X:
        return "Generic[5].X";
    case Attribute::Generic5Y:
        return "Generic[5].Y";
    case Attribute::Generic5Z:
        return "Generic[5].Z";
    case Attribute::Generic5W:
        return "Generic[5].W";
    case Attribute::Generic6X:
        return "Generic[6].X";
    case Attribute::Generic6Y:
        return "Generic[6].Y";
    case Attribute::Generic6Z:
        return "Generic[6].Z";
    case Attribute::Generic6W:
        return "Generic[6].W";
    case Attribute::Generic7X:
        return "Generic[7].X";
    case Attribute::Generic7Y:
        return "Generic[7].Y";
    case Attribute::Generic7Z:
        return "Generic[7].Z";
    case Attribute::Generic7W:
        return "Generic[7].W";
    case Attribute::Generic8X:
        return "Generic[8].X";
    case Attribute::Generic8Y:
        return "Generic[8].Y";
    case Attribute::Generic8Z:
        return "Generic[8].Z";
    case Attribute::Generic8W:
        return "Generic[8].W";
    case Attribute::Generic9X:
        return "Generic[9].X";
    case Attribute::Generic9Y:
        return "Generic[9].Y";
    case Attribute::Generic9Z:
        return "Generic[9].Z";
    case Attribute::Generic9W:
        return "Generic[9].W";
    case Attribute::Generic10X:
        return "Generic[10].X";
    case Attribute::Generic10Y:
        return "Generic[10].Y";
    case Attribute::Generic10Z:
        return "Generic[10].Z";
    case Attribute::Generic10W:
        return "Generic[10].W";
    case Attribute::Generic11X:
        return "Generic[11].X";
    case Attribute::Generic11Y:
        return "Generic[11].Y";
    case Attribute::Generic11Z:
        return "Generic[11].Z";
    case Attribute::Generic11W:
        return "Generic[11].W";
    case Attribute::Generic12X:
        return "Generic[12].X";
    case Attribute::Generic12Y:
        return "Generic[12].Y";
    case Attribute::Generic12Z:
        return "Generic[12].Z";
    case Attribute::Generic12W:
        return "Generic[12].W";
    case Attribute::Generic13X:
        return "Generic[13].X";
    case Attribute::Generic13Y:
        return "Generic[13].Y";
    case Attribute::Generic13Z:
        return "Generic[13].Z";
    case Attribute::Generic13W:
        return "Generic[13].W";
    case Attribute::Generic14X:
        return "Generic[14].X";
    case Attribute::Generic14Y:
        return "Generic[14].Y";
    case Attribute::Generic14Z:
        return "Generic[14].Z";
    case Attribute::Generic14W:
        return "Generic[14].W";
    case Attribute::Generic15X:
        return "Generic[15].X";
    case Attribute::Generic15Y:
        return "Generic[15].Y";
    case Attribute::Generic15Z:
        return "Generic[15].Z";
    case Attribute::Generic15W:
        return "Generic[15].W";
    case Attribute::Generic16X:
        return "Generic[16].X";
    case Attribute::Generic16Y:
        return "Generic[16].Y";
    case Attribute::Generic16Z:
        return "Generic[16].Z";
    case Attribute::Generic16W:
        return "Generic[16].W";
    case Attribute::Generic17X:
        return "Generic[17].X";
    case Attribute::Generic17Y:
        return "Generic[17].Y";
    case Attribute::Generic17Z:
        return "Generic[17].Z";
    case Attribute::Generic17W:
        return "Generic[17].W";
    case Attribute::Generic18X:
        return "Generic[18].X";
    case Attribute::Generic18Y:
        return "Generic[18].Y";
    case Attribute::Generic18Z:
        return "Generic[18].Z";
    case Attribute::Generic18W:
        return "Generic[18].W";
    case Attribute::Generic19X:
        return "Generic[19].X";
    case Attribute::Generic19Y:
        return "Generic[19].Y";
    case Attribute::Generic19Z:
        return "Generic[19].Z";
    case Attribute::Generic19W:
        return "Generic[19].W";
    case Attribute::Generic20X:
        return "Generic[20].X";
    case Attribute::Generic20Y:
        return "Generic[20].Y";
    case Attribute::Generic20Z:
        return "Generic[20].Z";
    case Attribute::Generic20W:
        return "Generic[20].W";
    case Attribute::Generic21X:
        return "Generic[21].X";
    case Attribute::Generic21Y:
        return "Generic[21].Y";
    case Attribute::Generic21Z:
        return "Generic[21].Z";
    case Attribute::Generic21W:
        return "Generic[21].W";
    case Attribute::Generic22X:
        return "Generic[22].X";
    case Attribute::Generic22Y:
        return "Generic[22].Y";
    case Attribute::Generic22Z:
        return "Generic[22].Z";
    case Attribute::Generic22W:
        return "Generic[22].W";
    case Attribute::Generic23X:
        return "Generic[23].X";
    case Attribute::Generic23Y:
        return "Generic[23].Y";
    case Attribute::Generic23Z:
        return "Generic[23].Z";
    case Attribute::Generic23W:
        return "Generic[23].W";
    case Attribute::Generic24X:
        return "Generic[24].X";
    case Attribute::Generic24Y:
        return "Generic[24].Y";
    case Attribute::Generic24Z:
        return "Generic[24].Z";
    case Attribute::Generic24W:
        return "Generic[24].W";
    case Attribute::Generic25X:
        return "Generic[25].X";
    case Attribute::Generic25Y:
        return "Generic[25].Y";
    case Attribute::Generic25Z:
        return "Generic[25].Z";
    case Attribute::Generic25W:
        return "Generic[25].W";
    case Attribute::Generic26X:
        return "Generic[26].X";
    case Attribute::Generic26Y:
        return "Generic[26].Y";
    case Attribute::Generic26Z:
        return "Generic[26].Z";
    case Attribute::Generic26W:
        return "Generic[26].W";
    case Attribute::Generic27X:
        return "Generic[27].X";
    case Attribute::Generic27Y:
        return "Generic[27].Y";
    case Attribute::Generic27Z:
        return "Generic[27].Z";
    case Attribute::Generic27W:
        return "Generic[27].W";
    case Attribute::Generic28X:
        return "Generic[28].X";
    case Attribute::Generic28Y:
        return "Generic[28].Y";
    case Attribute::Generic28Z:
        return "Generic[28].Z";
    case Attribute::Generic28W:
        return "Generic[28].W";
    case Attribute::Generic29X:
        return "Generic[29].X";
    case Attribute::Generic29Y:
        return "Generic[29].Y";
    case Attribute::Generic29Z:
        return "Generic[29].Z";
    case Attribute::Generic29W:
        return "Generic[29].W";
    case Attribute::Generic30X:
        return "Generic[30].X";
    case Attribute::Generic30Y:
        return "Generic[30].Y";
    case Attribute::Generic30Z:
        return "Generic[30].Z";
    case Attribute::Generic30W:
        return "Generic[30].W";
    case Attribute::Generic31X:
        return "Generic[31].X";
    case Attribute::Generic31Y:
        return "Generic[31].Y";
    case Attribute::Generic31Z:
        return "Generic[31].Z";
    case Attribute::Generic31W:
        return "Generic[31].W";
    case Attribute::ColorFrontDiffuseR:
        return "ColorFrontDiffuse.R";
    case Attribute::ColorFrontDiffuseG:
        return "ColorFrontDiffuse.G";
    case Attribute::ColorFrontDiffuseB:
        return "ColorFrontDiffuse.B";
    case Attribute::ColorFrontDiffuseA:
        return "ColorFrontDiffuse.A";
    case Attribute::ColorFrontSpecularR:
        return "ColorFrontSpecular.R";
    case Attribute::ColorFrontSpecularG:
        return "ColorFrontSpecular.G";
    case Attribute::ColorFrontSpecularB:
        return "ColorFrontSpecular.B";
    case Attribute::ColorFrontSpecularA:
        return "ColorFrontSpecular.A";
    case Attribute::ColorBackDiffuseR:
        return "ColorBackDiffuse.R";
    case Attribute::ColorBackDiffuseG:
        return "ColorBackDiffuse.G";
    case Attribute::ColorBackDiffuseB:
        return "ColorBackDiffuse.B";
    case Attribute::ColorBackDiffuseA:
        return "ColorBackDiffuse.A";
    case Attribute::ColorBackSpecularR:
        return "ColorBackSpecular.R";
    case Attribute::ColorBackSpecularG:
        return "ColorBackSpecular.G";
    case Attribute::ColorBackSpecularB:
        return "ColorBackSpecular.B";
    case Attribute::ColorBackSpecularA:
        return "ColorBackSpecular.A";
    case Attribute::ClipDistance0:
        return "ClipDistance[0]";
    case Attribute::ClipDistance1:
        return "ClipDistance[1]";
    case Attribute::ClipDistance2:
        return "ClipDistance[2]";
    case Attribute::ClipDistance3:
        return "ClipDistance[3]";
    case Attribute::ClipDistance4:
        return "ClipDistance[4]";
    case Attribute::ClipDistance5:
        return "ClipDistance[5]";
    case Attribute::ClipDistance6:
        return "ClipDistance[6]";
    case Attribute::ClipDistance7:
        return "ClipDistance[7]";
    case Attribute::PointSpriteS:
        return "PointSprite.S";
    case Attribute::PointSpriteT:
        return "PointSprite.T";
    case Attribute::FogCoordinate:
        return "FogCoordinate";
    case Attribute::TessellationEvaluationPointU:
        return "TessellationEvaluationPoint.U";
    case Attribute::TessellationEvaluationPointV:
        return "TessellationEvaluationPoint.V";
    case Attribute::InstanceId:
        return "InstanceId";
    case Attribute::VertexId:
        return "VertexId";
    case Attribute::FixedFncTexture0S:
        return "FixedFncTexture[0].S";
    case Attribute::FixedFncTexture0T:
        return "FixedFncTexture[0].T";
    case Attribute::FixedFncTexture0R:
        return "FixedFncTexture[0].R";
    case Attribute::FixedFncTexture0Q:
        return "FixedFncTexture[0].Q";
    case Attribute::FixedFncTexture1S:
        return "FixedFncTexture[1].S";
    case Attribute::FixedFncTexture1T:
        return "FixedFncTexture[1].T";
    case Attribute::FixedFncTexture1R:
        return "FixedFncTexture[1].R";
    case Attribute::FixedFncTexture1Q:
        return "FixedFncTexture[1].Q";
    case Attribute::FixedFncTexture2S:
        return "FixedFncTexture[2].S";
    case Attribute::FixedFncTexture2T:
        return "FixedFncTexture[2].T";
    case Attribute::FixedFncTexture2R:
        return "FixedFncTexture[2].R";
    case Attribute::FixedFncTexture2Q:
        return "FixedFncTexture[2].Q";
    case Attribute::FixedFncTexture3S:
        return "FixedFncTexture[3].S";
    case Attribute::FixedFncTexture3T:
        return "FixedFncTexture[3].T";
    case Attribute::FixedFncTexture3R:
        return "FixedFncTexture[3].R";
    case Attribute::FixedFncTexture3Q:
        return "FixedFncTexture[3].Q";
    case Attribute::FixedFncTexture4S:
        return "FixedFncTexture[4].S";
    case Attribute::FixedFncTexture4T:
        return "FixedFncTexture[4].T";
    case Attribute::FixedFncTexture4R:
        return "FixedFncTexture[4].R";
    case Attribute::FixedFncTexture4Q:
        return "FixedFncTexture[4].Q";
    case Attribute::FixedFncTexture5S:
        return "FixedFncTexture[5].S";
    case Attribute::FixedFncTexture5T:
        return "FixedFncTexture[5].T";
    case Attribute::FixedFncTexture5R:
        return "FixedFncTexture[5].R";
    case Attribute::FixedFncTexture5Q:
        return "FixedFncTexture[5].Q";
    case Attribute::FixedFncTexture6S:
        return "FixedFncTexture[6].S";
    case Attribute::FixedFncTexture6T:
        return "FixedFncTexture[6].T";
    case Attribute::FixedFncTexture6R:
        return "FixedFncTexture[6].R";
    case Attribute::FixedFncTexture6Q:
        return "FixedFncTexture[6].Q";
    case Attribute::FixedFncTexture7S:
        return "FixedFncTexture[7].S";
    case Attribute::FixedFncTexture7T:
        return "FixedFncTexture[7].T";
    case Attribute::FixedFncTexture7R:
        return "FixedFncTexture[7].R";
    case Attribute::FixedFncTexture7Q:
        return "FixedFncTexture[7].Q";
    case Attribute::FixedFncTexture8S:
        return "FixedFncTexture[8].S";
    case Attribute::FixedFncTexture8T:
        return "FixedFncTexture[8].T";
    case Attribute::FixedFncTexture8R:
        return "FixedFncTexture[8].R";
    case Attribute::FixedFncTexture8Q:
        return "FixedFncTexture[8].Q";
    case Attribute::FixedFncTexture9S:
        return "FixedFncTexture[9].S";
    case Attribute::FixedFncTexture9T:
        return "FixedFncTexture[9].T";
    case Attribute::FixedFncTexture9R:
        return "FixedFncTexture[9].R";
    case Attribute::FixedFncTexture9Q:
        return "FixedFncTexture[9].Q";
    case Attribute::ViewportMask:
        return "ViewportMask";
    case Attribute::FrontFace:
        return "FrontFace";
    }
    return fmt::format("<reserved attribute {}>", static_cast<int>(attribute));
}

} // namespace Shader::IR
