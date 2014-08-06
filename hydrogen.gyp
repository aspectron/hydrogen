{
    'conditions': [
        ['OS=="win"', {
            'variables': {
                'gl_include_dirs': ['extern/glew/include'],
                'gl_libraries': ['-lopengl32', 'extern/glew/lib/$(ShortPlatform)/glew32.lib'],
            },
        }],
        ['OS=="mac"', {
            'variables': {
                'gl_include_dirs': [],
                'gl_libraries': ['OpenGL.framework', 'AppKit.framework'],
            },
        }],
        ['OS=="linux"', {
            'variables': {
                'gl_include_dirs': [],
                'gl_libraries': ['-lGLEW'],
            },
        }],
    ],

    'variables': {
        'bullet_dir': 'extern/bullet/src',
    },

    'targets': [
        {
            'target_name': 'hydrogen',
            'type': 'shared_library',
            'msvs_guid': '0F581191-E1FC-407D-A535-738EED3C9216',
            'dependencies': [
                '<(jsx)/sdk/core/core.gyp:core',
                '<(jsx)/extern/extern.gyp:*',
                '<(jsx)/../image/image.gyp:image',
                '<(jsx)/../math/math.gyp:math',
                '<(jsx)/../oxygen/oxygen.gyp:oxygen',
                'bullet',
                'hydrogen_doc',
            ],
            'direct_dependent_settings': {
                'include_dirs': ['src', '<(bullet_dir)', '<@(gl_include_dirs)'],
                'libraries': ['<@(gl_libraries)'],
            },
            'defines': ['HYDROGEN_EXPORTS'],
            'include_dirs': [ '<@(gl_include_dirs)'],
            'libraries': ['<@(gl_libraries)'],
            'sources': [
                'src/gl.camera.cpp',
                'src/gl.camera.hpp',
                'src/gl.color.hpp',
                'src/gl.engine.cpp',
                'src/gl.engine.hpp',
                'src/gl.entity.cpp',
                'src/gl.entity.hpp',
                'src/gl.iface.hpp',
                'src/gl.shader.hpp',
                'src/gl.texture.cpp',
                'src/gl.texture.hpp',
                'src/gl.transform.hpp',
                'src/gl.viewport.hpp',
                'src/hydrogen.cpp',
                'src/hydrogen.hpp',
                'src/physics.bullet.cpp',
                'src/physics.bullet.hpp',
            ],
            'conditions': [
                ['OS=="win"', {
                    'sources': [
                        'src/gl.iface.wgl.cpp',
                        'src/gl.iface.wgl.hpp',
                    ],
                }],
                ['OS=="mac"', {
                    'sources': [
                        'src/gl.iface.nsgl.mm',
                        'src/gl.iface.nsgl.hpp',
                    ],
                }],
                ['OS=="linux"', {
                    'sources': [
                        'src/gl.iface.glx.cpp',
                        'src/gl.iface.glx.hpp',
                    ],
                }],
            ],
        },
        {
            'target_name': 'bullet',
            'type': 'static_library',
            'msvs_guid': '38CA01DD-8E64-41C3-9585-08FA77096317',
            'direct_dependent_settings': {
                'include_dirs': ['<(bullet_dir)'],
            },
            'include_dirs': ['<(bullet_dir)'],
            'cflags_cc': ['-Wno-c++11-narrowing'],
            'sources': [
                '<(bullet_dir)/btBulletCollisionCommon.h',
                '<(bullet_dir)/btBulletDynamicsCommon.h',

                '<(bullet_dir)/BulletCollision/BroadphaseCollision/btAxisSweep3.cpp',
                '<(bullet_dir)/BulletCollision/BroadphaseCollision/btBroadphaseProxy.cpp',
                '<(bullet_dir)/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.cpp',
                '<(bullet_dir)/BulletCollision/BroadphaseCollision/btDbvt.cpp',
                '<(bullet_dir)/BulletCollision/BroadphaseCollision/btDbvtBroadphase.cpp',
                '<(bullet_dir)/BulletCollision/BroadphaseCollision/btDispatcher.cpp',
                '<(bullet_dir)/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.cpp',
                '<(bullet_dir)/BulletCollision/BroadphaseCollision/btOverlappingPairCache.cpp',
                '<(bullet_dir)/BulletCollision/BroadphaseCollision/btQuantizedBvh.cpp',
                '<(bullet_dir)/BulletCollision/BroadphaseCollision/btSimpleBroadphase.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btBox2dBox2dCollisionAlgorithm.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btBoxBoxDetector.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btCollisionDispatcher.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btCollisionObject.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btCollisionWorld.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btCompoundCollisionAlgorithm.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btConvex2dConvex2dAlgorithm.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btEmptyCollisionAlgorithm.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btGhostObject.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btInternalEdgeUtility.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btManifoldResult.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btSimulationIslandManager.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btSphereBoxCollisionAlgorithm.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/btUnionFind.cpp',
                '<(bullet_dir)/BulletCollision/CollisionDispatch/SphereTriangleDetector.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btBox2dShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btBoxShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btCapsuleShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btCollisionShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btCompoundShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btConcaveShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btConeShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btConvex2dShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btConvexHullShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btConvexInternalShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btConvexPointCloudShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btConvexPolyhedron.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btConvexShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btCylinderShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btEmptyShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btMinkowskiSumShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btMultiSphereShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btOptimizedBvh.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btPolyhedralConvexShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btShapeHull.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btSphereShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btStaticPlaneShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btStridingMeshInterface.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btTetrahedronShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btTriangleBuffer.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btTriangleCallback.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btTriangleIndexVertexMaterialArray.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btTriangleMesh.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btTriangleMeshShape.cpp',
                '<(bullet_dir)/BulletCollision/CollisionShapes/btUniformScalingShape.cpp',
                '<(bullet_dir)/BulletCollision/Gimpact/btContactProcessing.cpp',
                '<(bullet_dir)/BulletCollision/Gimpact/btGenericPoolAllocator.cpp',
                '<(bullet_dir)/BulletCollision/Gimpact/btGImpactBvh.cpp',
                '<(bullet_dir)/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.cpp',
                '<(bullet_dir)/BulletCollision/Gimpact/btGImpactQuantizedBvh.cpp',
                '<(bullet_dir)/BulletCollision/Gimpact/btGImpactShape.cpp',
                '<(bullet_dir)/BulletCollision/Gimpact/btTriangleShapeEx.cpp',
                '<(bullet_dir)/BulletCollision/Gimpact/gim_box_set.cpp',
                '<(bullet_dir)/BulletCollision/Gimpact/gim_contact.cpp',
                '<(bullet_dir)/BulletCollision/Gimpact/gim_memory.cpp',
                '<(bullet_dir)/BulletCollision/Gimpact/gim_tri_collision.cpp',
                '<(bullet_dir)/BulletCollision/NarrowPhaseCollision/btContinuousConvexCollision.cpp',
                '<(bullet_dir)/BulletCollision/NarrowPhaseCollision/btConvexCast.cpp',
                '<(bullet_dir)/BulletCollision/NarrowPhaseCollision/btGjkConvexCast.cpp',
                '<(bullet_dir)/BulletCollision/NarrowPhaseCollision/btGjkEpa2.cpp',
                '<(bullet_dir)/BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.cpp',
                '<(bullet_dir)/BulletCollision/NarrowPhaseCollision/btGjkPairDetector.cpp',
                '<(bullet_dir)/BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.cpp',
                '<(bullet_dir)/BulletCollision/NarrowPhaseCollision/btPersistentManifold.cpp',
                '<(bullet_dir)/BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.cpp',
                '<(bullet_dir)/BulletCollision/NarrowPhaseCollision/btRaycastCallback.cpp',
                '<(bullet_dir)/BulletCollision/NarrowPhaseCollision/btSubSimplexConvexCast.cpp',
                '<(bullet_dir)/BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.cpp',
                '<(bullet_dir)/BulletDynamics/Character/btKinematicCharacterController.cpp',
                '<(bullet_dir)/BulletDynamics/ConstraintSolver/btConeTwistConstraint.cpp',
                '<(bullet_dir)/BulletDynamics/ConstraintSolver/btContactConstraint.cpp',
                '<(bullet_dir)/BulletDynamics/ConstraintSolver/btGearConstraint.cpp',
                '<(bullet_dir)/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.cpp',
                '<(bullet_dir)/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.cpp',
                '<(bullet_dir)/BulletDynamics/ConstraintSolver/btHinge2Constraint.cpp',
                '<(bullet_dir)/BulletDynamics/ConstraintSolver/btHingeConstraint.cpp',
                '<(bullet_dir)/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.cpp',
                '<(bullet_dir)/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.cpp',
                '<(bullet_dir)/BulletDynamics/ConstraintSolver/btSliderConstraint.cpp',
                '<(bullet_dir)/BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.cpp',
                '<(bullet_dir)/BulletDynamics/ConstraintSolver/btTypedConstraint.cpp',
                '<(bullet_dir)/BulletDynamics/ConstraintSolver/btUniversalConstraint.cpp',
                '<(bullet_dir)/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.cpp',
                '<(bullet_dir)/BulletDynamics/Dynamics/btRigidBody.cpp',
                '<(bullet_dir)/BulletDynamics/Dynamics/btSimpleDynamicsWorld.cpp',
                '<(bullet_dir)/BulletDynamics/Vehicle/btRaycastVehicle.cpp',
                '<(bullet_dir)/BulletDynamics/Vehicle/btWheelInfo.cpp',
                '<(bullet_dir)/BulletMultiThreaded/btGpu3DGridBroadphase.cpp',
                '<(bullet_dir)/BulletMultiThreaded/btParallelConstraintSolver.cpp',
                '<(bullet_dir)/BulletMultiThreaded/btThreadSupportInterface.cpp',
                '<(bullet_dir)/BulletMultiThreaded/PosixThreadSupport.cpp',
                '<(bullet_dir)/BulletMultiThreaded/SequentialThreadSupport.cpp',
                '<(bullet_dir)/BulletMultiThreaded/SpuCollisionObjectWrapper.cpp',
                '<(bullet_dir)/BulletMultiThreaded/SpuCollisionTaskProcess.cpp',
                '<(bullet_dir)/BulletMultiThreaded/SpuContactManifoldCollisionAlgorithm.cpp',
                '<(bullet_dir)/BulletMultiThreaded/SpuFakeDma.cpp',
                '<(bullet_dir)/BulletMultiThreaded/SpuGatheringCollisionDispatcher.cpp',
                '<(bullet_dir)/BulletMultiThreaded/SpuLibspe2Support.cpp',
                '<(bullet_dir)/BulletMultiThreaded/SpuSampleTaskProcess.cpp',
                '<(bullet_dir)/BulletMultiThreaded/Win32ThreadSupport.cpp',
#                '<(bullet_dir)/BulletMultiThreaded/GpuSoftBodySolvers/DX11/btSoftBodySolver_DX11.cpp',
#                '<(bullet_dir)/BulletMultiThreaded/GpuSoftBodySolvers/DX11/btSoftBodySolver_DX11SIMDAware.cpp',
#                '<(bullet_dir)/BulletMultiThreaded/GpuSoftBodySolvers/OpenCL/btSoftBodySolverOutputCLtoGL.cpp',
#                '<(bullet_dir)/BulletMultiThreaded/GpuSoftBodySolvers/OpenCL/btSoftBodySolver_OpenCL.cpp',
#                '<(bullet_dir)/BulletMultiThreaded/GpuSoftBodySolvers/OpenCL/btSoftBodySolver_OpenCLSIMDAware.cpp',
#                '<(bullet_dir)/BulletMultiThreaded/GpuSoftBodySolvers/OpenCL/MiniCL/MiniCLTaskWrap.cpp',
                '<(bullet_dir)/BulletMultiThreaded/SpuNarrowPhaseCollisionTask/boxBoxDistance.cpp',
                '<(bullet_dir)/BulletMultiThreaded/SpuNarrowPhaseCollisionTask/SpuCollisionShapes.cpp',
                '<(bullet_dir)/BulletMultiThreaded/SpuNarrowPhaseCollisionTask/SpuContactResult.cpp',
                '<(bullet_dir)/BulletMultiThreaded/SpuNarrowPhaseCollisionTask/SpuGatheringCollisionTask.cpp',
                '<(bullet_dir)/BulletMultiThreaded/SpuNarrowPhaseCollisionTask/SpuMinkowskiPenetrationDepthSolver.cpp',
                '<(bullet_dir)/BulletMultiThreaded/SpuSampleTask/SpuSampleTask.cpp',
                '<(bullet_dir)/BulletSoftBody/btDefaultSoftBodySolver.cpp',
                '<(bullet_dir)/BulletSoftBody/btSoftBody.cpp',
                '<(bullet_dir)/BulletSoftBody/btSoftBodyConcaveCollisionAlgorithm.cpp',
                '<(bullet_dir)/BulletSoftBody/btSoftBodyHelpers.cpp',
                '<(bullet_dir)/BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.cpp',
                '<(bullet_dir)/BulletSoftBody/btSoftRigidCollisionAlgorithm.cpp',
                '<(bullet_dir)/BulletSoftBody/btSoftRigidDynamicsWorld.cpp',
                '<(bullet_dir)/BulletSoftBody/btSoftSoftCollisionAlgorithm.cpp',
                '<(bullet_dir)/LinearMath/btAlignedAllocator.cpp',
                '<(bullet_dir)/LinearMath/btConvexHull.cpp',
                '<(bullet_dir)/LinearMath/btConvexHullComputer.cpp',
                '<(bullet_dir)/LinearMath/btGeometryUtil.cpp',
                '<(bullet_dir)/LinearMath/btPolarDecomposition.cpp',
                '<(bullet_dir)/LinearMath/btQuickprof.cpp',
                '<(bullet_dir)/LinearMath/btSerializer.cpp',
                '<(bullet_dir)/LinearMath/btVector3.cpp',
                '<(bullet_dir)/MiniCL/MiniCL.cpp',
                '<(bullet_dir)/MiniCL/MiniCLTaskScheduler.cpp',
                '<(bullet_dir)/MiniCL/MiniCLTask/MiniCLTask.cpp',
            ],
        },
        {
            'target_name': 'hydrogen_doc',
            'type': 'none',
            'dependencies': [
                '<(jsx)/apps/jsx/jsx.gyp:*',
            ],

            'variables': {
                'jsx_app': '<(PRODUCT_DIR)/<(EXECUTABLE_PREFIX)jsx<(EXECUTABLE_SUFFIX)',
                'doc_dir': 'doc/hydrogen',
            },
            'conditions': [
                ['OS=="linux"', {
                    'variables': { 'jsx_app': '<(out_dir)/<(EXECUTABLE_PREFIX)jsx<(EXECUTABLE_SUFFIX)' },
                }],
            ],

            'actions': [
                {
                    'action_name': 'build_doc',
                    'inputs': ['rte/hydrogen.js', 'src/hydrogen.cpp'],
                    'outputs': ['<(doc_dir)/all.md'],
                    'action': ['<(jsx_app)', '<(jsx)/build/tools/gendoc/run.js',
                        '<(doc_dir)', 'rte/.+[.]js', 'src/.+[.]cpp',
                    ],
                    'msvs_cygwin_shell': 0,
                    'message': 'Building documentation...',
                },
            ],
        },
    ],
}
