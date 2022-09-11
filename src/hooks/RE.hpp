#pragma once

#include "RE/H/hkVector4.h"
#include "RE/H/hkpConvexShape.h"
#include <ehdata.h>
#include <rttidata.h>
namespace RE
{
	std::string GetRawName(void* obj) {
		// Get the meta entry in vftable
		_RTTICompleteObjectLocator* col = reinterpret_cast<_RTTICompleteObjectLocator***>(obj)[0][-1];

		// Calculate image base by subtracting the RTTICompleteObjectLocator's pSelf offset from RTTICompleteObjectLocator's pointer
		uintptr_t imageBase = reinterpret_cast<uintptr_t>(col) - col->pSelf;

		// Get the type descriptor by adding TypeDescriptor's offset to the image base
		TypeDescriptor* tDesc = reinterpret_cast<TypeDescriptor*>(imageBase + col->pTypeDescriptor);

		// At the end, we can get the type's mangled name
		const char* colName = tDesc->name;

	}
	class hkpTriangleShape : public hkpConvexShape
	{
		public:
			inline static constexpr auto RTTI = RTTI_hkpTriangleShape;

			~hkpTriangleShape() override;  // 00

			// override (hkpConvexShape)
			void                     CalcContentStatistics(hkStatisticsCollector* a_collector, const hkClass* a_class) const override;     // 02
			std::int32_t             CalcSizeForSpu(const CalcSizeForSpuInput& a_input, std::int32_t a_spuBufferSizeLeft) const override;  // 06
			void                     GetAabbImpl(const hkTransform& a_localToWorld, float a_tolerance, hkAabb& a_out) const override;      // 07
			bool                     CastRayImpl(const hkpShapeRayCastInput& a_input, hkpShapeRayCastOutput& a_output) const override;     // 08
			hkVector4Comparison      CastRayBundleImpl(const hkpShapeRayBundleCastInput& a_input, hkpShapeRayBundleCastOutput& a_output, const hkVector4Comparison& a_mask) const override;  // 0A
			std::int32_t             GetNumCollisionSpheresImpl() override;                                                                // 0B
			const hkpSphere*         GetCollisionSpheresImpl(hkSphere* a_sphereBuffer) override;                                           // 0C

			// members
			std::uint16_t weldingInfo;
			stl::enumeration<hkpWeldingUtility::WeldingType, std::uint8_t>       weldingType;
			std::uint8_t isExtruded;
			hkVector4 vertexA;
			hkVector4 vertexB;
			hkVector4 vertexC;
			hkVector4 extrusion;
	};

	class hkpConvexTransformShapeBase : public hkpConvexShape {
		public:
			inline static constexpr auto RTTI = RTTI_hkpConvexTransformShapeBase;

			~hkpConvexTransformShapeBase() override;  // 00

			hkpSingleShapeContainer childShape;
			mutable std::int32_t childSize;
	};

	class hkpConvexTransformShape : public hkpConvexTransformShapeBase
	{
		public:
			inline static constexpr auto RTTI = RTTI_hkpConvexTransformShape;

			~hkpConvexTransformShape() override;  // 00

			// override (hkpConvexShape)
			void                     CalcContentStatistics(hkStatisticsCollector* a_collector, const hkClass* a_class) const override;     // 02
			const hkpShapeContainer* GetContainer() const override;                                                                        // 04
			std::int32_t             CalcSizeForSpu(const CalcSizeForSpuInput& a_input, std::int32_t a_spuBufferSizeLeft) const override;  // 06
			void                     GetAabbImpl(const hkTransform& a_localToWorld, float a_tolerance, hkAabb& a_out) const override;      // 07
			bool                     CastRayImpl(const hkpShapeRayCastInput& a_input, hkpShapeRayCastOutput& a_output) const override;     // 08
			void                     CastRayWithCollectorImpl(const hkpShapeRayCastInput& a_input, const hkpCdBody& a_cdBody, hkpRayHitCollector& a_collector) const override;               // 09
			std::int32_t             GetNumCollisionSpheresImpl() override;                                                                // 0B
			const hkpSphere*         GetCollisionSpheresImpl(hkSphere* a_sphereBuffer) override;                                           // 0C

			// members
			hkQsTransform transform;
			hkVector4 extraScale;
	};

	class hkFourTransposedPoints
	{
		public:
			hkVector4 vertices[3];
	};

	class hkpConvexVerticesConnectivity : public hkReferencedObject {
		public:
			inline static constexpr auto RTTI = RTTI_hkpConvexVerticesConnectivity;

			~hkpConvexVerticesConnectivity() override;  // 00

			virtual void clear();

			hkArray<std::uint16_t> vertexIndices;

			hkArray<std::uint8_t> numVerticesPerFace;
	};

	class hkpConvexVerticesShape : public hkpConvexShape
	{
		public:
			inline static constexpr auto RTTI = RTTI_hkpConvexVerticesShape;

			~hkpConvexVerticesShape() override;  // 00

			// override (hkpConvexShape)
			void                     CalcContentStatistics(hkStatisticsCollector* a_collector, const hkClass* a_class) const override;     // 02
			std::int32_t             CalcSizeForSpu(const CalcSizeForSpuInput& a_input, std::int32_t a_spuBufferSizeLeft) const override;  // 06
			void                     GetAabbImpl(const hkTransform& a_localToWorld, float a_tolerance, hkAabb& a_out) const override;      // 07
			bool                     CastRayImpl(const hkpShapeRayCastInput& a_input, hkpShapeRayCastOutput& a_output) const override;     // 08
			std::int32_t             GetNumCollisionSpheresImpl() override;                                                                // 0B
			const hkpSphere*         GetCollisionSpheresImpl(hkSphere* a_sphereBuffer) override;                                           // 0C

			// members
			// Cached AABB
			hkVector4 aabbHalfExtents;
			hkVector4 aabbCenter;

			hkArray<hkFourTransposedPoints> rotatedVertices;
			std::int32_t numVertices;

			bool useSpuBuffer;

			mutable hkArray<hkVector4> planeEquations;

			// FYI: Cannot get connectivity to work
			mutable hkpConvexVerticesConnectivity* connectivity;
	};


	class hkbCharacterController : public hkReferencedObject
	{
		public:

			~hkbCharacterController() override;

			/// The character driver will call this function to integrate the character controller.
			/// The character controller need not update it's position immediately if isIntegratedWithPhysics() == true.
			/// But the position should be updated after hkbPhysicsInterface::step is called.
			virtual void integrate( hkVector4 newVelocity, float timestep ) = 0;

			/// Indicates if the controller is integrated as part of hkbPhysicsInterface::step.
			virtual bool isIntegratedWithPhysics() const = 0;

			/// The current position of the character controller.
			virtual void getPosition( hkVector4& positionOut ) const = 0;

			/// The current linear velocity of the character controller.
			virtual void getLinearVelocity( hkVector4& linearVelocityOut ) const = 0;

			/// Checks if the character controller is currently supported in the down direction, this is called once per frame
			/// after the character is integrated and hkbPhysicsInterface::step is (possibly) called. If the character is
			/// supported it should return true and fill out supportingNormalOut.
			virtual bool checkSupport( hkVector4 down, float timestep, hkVector4& supportingNormalOut ) = 0;

			/// Sets the collision filter info dynamically (after creation)
			virtual void setCollisionFilterInfo( std::uint32_t filterInfo ) = 0;
	};

	class hkbpCharacterController : public hkbCharacterController
	{
		public:

			~hkbpCharacterController() override;

			/// Get the hkpCollidable used by this character controller.
			virtual const hkpCollidable* getCollidable() const = 0;
	};

	/// This is a wrapper for a Havok Physics2012 proxy-based character controller.
	class hkbpCharacterProxyController : public hkbpCharacterController
	{
		public:

			virtual ~hkbpCharacterProxyController() override;


			// hkbCharacterController interface
			virtual void integrate( hkVector4 newVelocity, float timestep ) override;

			// hkbCharacterController interface
			virtual bool isIntegratedWithPhysics() const override;

			// hkbCharacterController interface
			virtual void getPosition( hkVector4& positionOut ) const override;

			// hkbCharacterController interface
			virtual void getLinearVelocity( hkVector4& positionOut ) const override;

			// hkbCharacterController interface
			virtual bool checkSupport( hkVector4 down, float timestep, hkVector4& supportingNormalOut ) override;

			// hkbCharacterContoller interface
			virtual void setCollisionFilterInfo( std::uint32_t filterInfo ) override;

			//////////////////////////////////////////////////////////////////////////
			// hkbpCharacterController interface
			//////////////////////////////////////////////////////////////////////////

			// hkbpCharacterController interface
			virtual const hkpCollidable* getCollidable() const override;


			/// The proxy associated with the character.
			hkRefPtr<hkpCharacterProxy> m_characterProxy;
	};

	class hkbCharacterControllerDriver : public hkReferencedObject
	{
		public:
			~hkbCharacterControllerDriver()  override;  // 00

			// The current controller (maybe null).
			hkRefPtr<hkbCharacterController> m_controller;

			// The expected or anticipated position of the controller after it is integrated.
			hkVector4 m_expectedPosition;

			// The horizontal displacement used to calculate the final position.
			hkVector4 m_horizontalDisplacement;

			// The vertical displacement used to calculate the final position.
			float m_verticalDisplacement;

			// Indicating the controller reported it was supported.
			bool m_isSupported;

			// Stored supporting normal from the controller.
			hkVector4 m_supportingNormal;

			// Stored from current character controller
			hkVector4 m_storedPosition;

			// Stored from current character controller
			hkVector4 m_storedLinearVelocity;

			// Current enabled status.
			bool m_isEnabled;
	};
}
