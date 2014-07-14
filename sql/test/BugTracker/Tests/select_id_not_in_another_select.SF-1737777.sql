CREATE TABLE "Match"(
	objID1 bigint NOT NULL,
	objID2 bigint NOT NULL,
	run1 smallint NOT NULL,
	run2 smallint NOT NULL,
	type1 tinyint NOT NULL,
	type2 tinyint NOT NULL,
	mode1 tinyint NOT NULL,
	mode2 tinyint NOT NULL,
	distance float NOT NULL,
	miss char(1) NOT NULL,
	matchHead bigint NOT NULL,
 CONSTRAINT pk_Match_objID1_objID2 PRIMARY KEY  
(
	objID1,
	objID2
)
);
CREATE TABLE PhotoObjAll(
	objID bigint NOT NULL,
	skyVersion tinyint NOT NULL,
	run smallint NOT NULL,
	rerun smallint NOT NULL,
	camcol tinyint NOT NULL,
	field smallint NOT NULL,
	obj smallint NOT NULL,
	mode tinyint NOT NULL,
	nChild smallint NOT NULL,
	type smallint NOT NULL,
	probPSF real NOT NULL,
	insideMask tinyint NOT NULL,
	flags bigint NOT NULL,
	rowc real NOT NULL,
	rowcErr real NOT NULL,
	colc real NOT NULL,
	colcErr real NOT NULL,
	rowv real NOT NULL,
	rowvErr real NOT NULL,
	colv real NOT NULL,
	colvErr real NOT NULL,
	rowc_u real NOT NULL,
	rowc_g real NOT NULL,
	rowc_r real NOT NULL,
	rowc_i real NOT NULL,
	rowc_z real NOT NULL,
	rowcErr_u real NOT NULL,
	rowcErr_g real NOT NULL,
	rowcErr_r real NOT NULL,
	rowcErr_i real NOT NULL,
	rowcErr_z real NOT NULL,
	colc_u real NOT NULL,
	colc_g real NOT NULL,
	colc_r real NOT NULL,
	colc_i real NOT NULL,
	colc_z real NOT NULL,
	colcErr_u real NOT NULL,
	colcErr_g real NOT NULL,
	colcErr_r real NOT NULL,
	colcErr_i real NOT NULL,
	colcErr_z real NOT NULL,
	sky_u real NOT NULL,
	sky_g real NOT NULL,
	sky_r real NOT NULL,
	sky_i real NOT NULL,
	sky_z real NOT NULL,
	skyErr_u real NOT NULL,
	skyErr_g real NOT NULL,
	skyErr_r real NOT NULL,
	skyErr_i real NOT NULL,
	skyErr_z real NOT NULL,
	psfMag_u real NOT NULL,
	psfMag_g real NOT NULL,
	psfMag_r real NOT NULL,
	psfMag_i real NOT NULL,
	psfMag_z real NOT NULL,
	psfMagErr_u real NOT NULL,
	psfMagErr_g real NOT NULL,
	psfMagErr_r real NOT NULL,
	psfMagErr_i real NOT NULL,
	psfMagErr_z real NOT NULL,
	fiberMag_u real NOT NULL,
	fiberMag_g real NOT NULL,
	fiberMag_r real NOT NULL,
	fiberMag_i real NOT NULL,
	fiberMag_z real NOT NULL,
	fiberMagErr_u real NOT NULL,
	fiberMagErr_g real NOT NULL,
	fiberMagErr_r real NOT NULL,
	fiberMagErr_i real NOT NULL,
	fiberMagErr_z real NOT NULL,
	petroMag_u real NOT NULL,
	petroMag_g real NOT NULL,
	petroMag_r real NOT NULL,
	petroMag_i real NOT NULL,
	petroMag_z real NOT NULL,
	petroMagErr_u real NOT NULL,
	petroMagErr_g real NOT NULL,
	petroMagErr_r real NOT NULL,
	petroMagErr_i real NOT NULL,
	petroMagErr_z real NOT NULL,
	petroRad_u real NOT NULL,
	petroRad_g real NOT NULL,
	petroRad_r real NOT NULL,
	petroRad_i real NOT NULL,
	petroRad_z real NOT NULL,
	petroRadErr_u real NOT NULL,
	petroRadErr_g real NOT NULL,
	petroRadErr_r real NOT NULL,
	petroRadErr_i real NOT NULL,
	petroRadErr_z real NOT NULL,
	petroR50_u real NOT NULL,
	petroR50_g real NOT NULL,
	petroR50_r real NOT NULL,
	petroR50_i real NOT NULL,
	petroR50_z real NOT NULL,
	petroR50Err_u real NOT NULL,
	petroR50Err_g real NOT NULL,
	petroR50Err_r real NOT NULL,
	petroR50Err_i real NOT NULL,
	petroR50Err_z real NOT NULL,
	petroR90_u real NOT NULL,
	petroR90_g real NOT NULL,
	petroR90_r real NOT NULL,
	petroR90_i real NOT NULL,
	petroR90_z real NOT NULL,
	petroR90Err_u real NOT NULL,
	petroR90Err_g real NOT NULL,
	petroR90Err_r real NOT NULL,
	petroR90Err_i real NOT NULL,
	petroR90Err_z real NOT NULL,
	q_u real NOT NULL,
	q_g real NOT NULL,
	q_r real NOT NULL,
	q_i real NOT NULL,
	q_z real NOT NULL,
	qErr_u real NOT NULL,
	qErr_g real NOT NULL,
	qErr_r real NOT NULL,
	qErr_i real NOT NULL,
	qErr_z real NOT NULL,
	u_u real NOT NULL,
	u_g real NOT NULL,
	u_r real NOT NULL,
	u_i real NOT NULL,
	u_z real NOT NULL,
	uErr_u real NOT NULL,
	uErr_g real NOT NULL,
	uErr_r real NOT NULL,
	uErr_i real NOT NULL,
	uErr_z real NOT NULL,
	mE1_u real NOT NULL,
	mE1_g real NOT NULL,
	mE1_r real NOT NULL,
	mE1_i real NOT NULL,
	mE1_z real NOT NULL,
	mE2_u real NOT NULL,
	mE2_g real NOT NULL,
	mE2_r real NOT NULL,
	mE2_i real NOT NULL,
	mE2_z real NOT NULL,
	mE1E1Err_u real NOT NULL,
	mE1E1Err_g real NOT NULL,
	mE1E1Err_r real NOT NULL,
	mE1E1Err_i real NOT NULL,
	mE1E1Err_z real NOT NULL,
	mE1E2Err_u real NOT NULL,
	mE1E2Err_g real NOT NULL,
	mE1E2Err_r real NOT NULL,
	mE1E2Err_i real NOT NULL,
	mE1E2Err_z real NOT NULL,
	mE2E2Err_u real NOT NULL,
	mE2E2Err_g real NOT NULL,
	mE2E2Err_r real NOT NULL,
	mE2E2Err_i real NOT NULL,
	mE2E2Err_z real NOT NULL,
	mRrCc_u real NOT NULL,
	mRrCc_g real NOT NULL,
	mRrCc_r real NOT NULL,
	mRrCc_i real NOT NULL,
	mRrCc_z real NOT NULL,
	mRrCcErr_u real NOT NULL,
	mRrCcErr_g real NOT NULL,
	mRrCcErr_r real NOT NULL,
	mRrCcErr_i real NOT NULL,
	mRrCcErr_z real NOT NULL,
	mCr4_u real NOT NULL,
	mCr4_g real NOT NULL,
	mCr4_r real NOT NULL,
	mCr4_i real NOT NULL,
	mCr4_z real NOT NULL,
	mE1PSF_u real NOT NULL,
	mE1PSF_g real NOT NULL,
	mE1PSF_r real NOT NULL,
	mE1PSF_i real NOT NULL,
	mE1PSF_z real NOT NULL,
	mE2PSF_u real NOT NULL,
	mE2PSF_g real NOT NULL,
	mE2PSF_r real NOT NULL,
	mE2PSF_i real NOT NULL,
	mE2PSF_z real NOT NULL,
	mRrCcPSF_u real NOT NULL,
	mRrCcPSF_g real NOT NULL,
	mRrCcPSF_r real NOT NULL,
	mRrCcPSF_i real NOT NULL,
	mRrCcPSF_z real NOT NULL,
	mCr4PSF_u real NOT NULL,
	mCr4PSF_g real NOT NULL,
	mCr4PSF_r real NOT NULL,
	mCr4PSF_i real NOT NULL,
	mCr4PSF_z real NOT NULL,
	isoRowc_u real NOT NULL,
	isoRowc_g real NOT NULL,
	isoRowc_r real NOT NULL,
	isoRowc_i real NOT NULL,
	isoRowc_z real NOT NULL,
	isoRowcErr_u real NOT NULL,
	isoRowcErr_g real NOT NULL,
	isoRowcErr_r real NOT NULL,
	isoRowcErr_i real NOT NULL,
	isoRowcErr_z real NOT NULL,
	isoRowcGrad_u real NOT NULL,
	isoRowcGrad_g real NOT NULL,
	isoRowcGrad_r real NOT NULL,
	isoRowcGrad_i real NOT NULL,
	isoRowcGrad_z real NOT NULL,
	isoColc_u real NOT NULL,
	isoColc_g real NOT NULL,
	isoColc_r real NOT NULL,
	isoColc_i real NOT NULL,
	isoColc_z real NOT NULL,
	isoColcErr_u real NOT NULL,
	isoColcErr_g real NOT NULL,
	isoColcErr_r real NOT NULL,
	isoColcErr_i real NOT NULL,
	isoColcErr_z real NOT NULL,
	isoColcGrad_u real NOT NULL,
	isoColcGrad_g real NOT NULL,
	isoColcGrad_r real NOT NULL,
	isoColcGrad_i real NOT NULL,
	isoColcGrad_z real NOT NULL,
	isoA_u real NOT NULL,
	isoA_g real NOT NULL,
	isoA_r real NOT NULL,
	isoA_i real NOT NULL,
	isoA_z real NOT NULL,
	isoAErr_u real NOT NULL,
	isoAErr_g real NOT NULL,
	isoAErr_r real NOT NULL,
	isoAErr_i real NOT NULL,
	isoAErr_z real NOT NULL,
	isoB_u real NOT NULL,
	isoB_g real NOT NULL,
	isoB_r real NOT NULL,
	isoB_i real NOT NULL,
	isoB_z real NOT NULL,
	isoBErr_u real NOT NULL,
	isoBErr_g real NOT NULL,
	isoBErr_r real NOT NULL,
	isoBErr_i real NOT NULL,
	isoBErr_z real NOT NULL,
	isoAGrad_u real NOT NULL,
	isoAGrad_g real NOT NULL,
	isoAGrad_r real NOT NULL,
	isoAGrad_i real NOT NULL,
	isoAGrad_z real NOT NULL,
	isoBGrad_u real NOT NULL,
	isoBGrad_g real NOT NULL,
	isoBGrad_r real NOT NULL,
	isoBGrad_i real NOT NULL,
	isoBGrad_z real NOT NULL,
	isoPhi_u real NOT NULL,
	isoPhi_g real NOT NULL,
	isoPhi_r real NOT NULL,
	isoPhi_i real NOT NULL,
	isoPhi_z real NOT NULL,
	isoPhiErr_u real NOT NULL,
	isoPhiErr_g real NOT NULL,
	isoPhiErr_r real NOT NULL,
	isoPhiErr_i real NOT NULL,
	isoPhiErr_z real NOT NULL,
	isoPhiGrad_u real NOT NULL,
	isoPhiGrad_g real NOT NULL,
	isoPhiGrad_r real NOT NULL,
	isoPhiGrad_i real NOT NULL,
	isoPhiGrad_z real NOT NULL,
	deVRad_u real NOT NULL,
	deVRad_g real NOT NULL,
	deVRad_r real NOT NULL,
	deVRad_i real NOT NULL,
	deVRad_z real NOT NULL,
	deVRadErr_u real NOT NULL,
	deVRadErr_g real NOT NULL,
	deVRadErr_r real NOT NULL,
	deVRadErr_i real NOT NULL,
	deVRadErr_z real NOT NULL,
	deVAB_u real NOT NULL,
	deVAB_g real NOT NULL,
	deVAB_r real NOT NULL,
	deVAB_i real NOT NULL,
	deVAB_z real NOT NULL,
	deVABErr_u real NOT NULL,
	deVABErr_g real NOT NULL,
	deVABErr_r real NOT NULL,
	deVABErr_i real NOT NULL,
	deVABErr_z real NOT NULL,
	deVPhi_u real NOT NULL,
	deVPhi_g real NOT NULL,
	deVPhi_r real NOT NULL,
	deVPhi_i real NOT NULL,
	deVPhi_z real NOT NULL,
	deVPhiErr_u real NOT NULL,
	deVPhiErr_g real NOT NULL,
	deVPhiErr_r real NOT NULL,
	deVPhiErr_i real NOT NULL,
	deVPhiErr_z real NOT NULL,
	deVMag_u real NOT NULL,
	deVMag_g real NOT NULL,
	deVMag_r real NOT NULL,
	deVMag_i real NOT NULL,
	deVMag_z real NOT NULL,
	deVMagErr_u real NOT NULL,
	deVMagErr_g real NOT NULL,
	deVMagErr_r real NOT NULL,
	deVMagErr_i real NOT NULL,
	deVMagErr_z real NOT NULL,
	expRad_u real NOT NULL,
	expRad_g real NOT NULL,
	expRad_r real NOT NULL,
	expRad_i real NOT NULL,
	expRad_z real NOT NULL,
	expRadErr_u real NOT NULL,
	expRadErr_g real NOT NULL,
	expRadErr_r real NOT NULL,
	expRadErr_i real NOT NULL,
	expRadErr_z real NOT NULL,
	expAB_u real NOT NULL,
	expAB_g real NOT NULL,
	expAB_r real NOT NULL,
	expAB_i real NOT NULL,
	expAB_z real NOT NULL,
	expABErr_u real NOT NULL,
	expABErr_g real NOT NULL,
	expABErr_r real NOT NULL,
	expABErr_i real NOT NULL,
	expABErr_z real NOT NULL,
	expPhi_u real NOT NULL,
	expPhi_g real NOT NULL,
	expPhi_r real NOT NULL,
	expPhi_i real NOT NULL,
	expPhi_z real NOT NULL,
	expPhiErr_u real NOT NULL,
	expPhiErr_g real NOT NULL,
	expPhiErr_r real NOT NULL,
	expPhiErr_i real NOT NULL,
	expPhiErr_z real NOT NULL,
	expMag_u real NOT NULL,
	expMag_g real NOT NULL,
	expMag_r real NOT NULL,
	expMag_i real NOT NULL,
	expMag_z real NOT NULL,
	expMagErr_u real NOT NULL,
	expMagErr_g real NOT NULL,
	expMagErr_r real NOT NULL,
	expMagErr_i real NOT NULL,
	expMagErr_z real NOT NULL,
	modelMag_u real NOT NULL,
	modelMag_g real NOT NULL,
	modelMag_r real NOT NULL,
	modelMag_i real NOT NULL,
	modelMag_z real NOT NULL,
	modelMagErr_u real NOT NULL,
	modelMagErr_g real NOT NULL,
	modelMagErr_r real NOT NULL,
	modelMagErr_i real NOT NULL,
	modelMagErr_z real NOT NULL,
	texture_u real NOT NULL,
	texture_g real NOT NULL,
	texture_r real NOT NULL,
	texture_i real NOT NULL,
	texture_z real NOT NULL,
	lnLStar_u real NOT NULL,
	lnLStar_g real NOT NULL,
	lnLStar_r real NOT NULL,
	lnLStar_i real NOT NULL,
	lnLStar_z real NOT NULL,
	lnLExp_u real NOT NULL,
	lnLExp_g real NOT NULL,
	lnLExp_r real NOT NULL,
	lnLExp_i real NOT NULL,
	lnLExp_z real NOT NULL,
	lnLDeV_u real NOT NULL,
	lnLDeV_g real NOT NULL,
	lnLDeV_r real NOT NULL,
	lnLDeV_i real NOT NULL,
	lnLDeV_z real NOT NULL,
	fracDeV_u real NOT NULL,
	fracDeV_g real NOT NULL,
	fracDeV_r real NOT NULL,
	fracDeV_i real NOT NULL,
	fracDeV_z real NOT NULL,
	flags_u bigint NOT NULL,
	flags_g bigint NOT NULL,
	flags_r bigint NOT NULL,
	flags_i bigint NOT NULL,
	flags_z bigint NOT NULL,
	type_u int NOT NULL,
	type_g int NOT NULL,
	type_r int NOT NULL,
	type_i int NOT NULL,
	type_z int NOT NULL,
	probPSF_u real NOT NULL,
	probPSF_g real NOT NULL,
	probPSF_r real NOT NULL,
	probPSF_i real NOT NULL,
	probPSF_z real NOT NULL,
	status int NOT NULL,
	ra float NOT NULL,
	"dec" float NOT NULL,
	cx float NOT NULL,
	cy float NOT NULL,
	cz float NOT NULL,
	offsetRa_u real NOT NULL,
	offsetRa_g real NOT NULL,
	offsetRa_r real NOT NULL,
	offsetRa_i real NOT NULL,
	offsetRa_z real NOT NULL,
	offsetDec_u real NOT NULL,
	offsetDec_g real NOT NULL,
	offsetDec_r real NOT NULL,
	offsetDec_i real NOT NULL,
	offsetDec_z real NOT NULL,
	primTarget int NOT NULL,
	secTarget int NOT NULL,
	extinction_u real NOT NULL,
	extinction_g real NOT NULL,
	extinction_r real NOT NULL,
	extinction_i real NOT NULL,
	extinction_z real NOT NULL,
	priority int NOT NULL,
	rho real NOT NULL,
	nProf_u int NOT NULL,
	nProf_g int NOT NULL,
	nProf_r int NOT NULL,
	nProf_i int NOT NULL,
	nProf_z int NOT NULL,
	loadVersion int NOT NULL,
	htmID bigint NOT NULL,
	fieldID bigint NOT NULL,
	parentID bigint NOT NULL DEFAULT (0),
	specObjID bigint NOT NULL DEFAULT (0),
	u real NOT NULL DEFAULT ((-9999)),
	g real NOT NULL DEFAULT ((-9999)),
	r real NOT NULL DEFAULT ((-9999)),
	i real NOT NULL DEFAULT ((-9999)),
	z real NOT NULL DEFAULT ((-9999)),
	err_u real NOT NULL DEFAULT ((-9999)),
	err_g real NOT NULL DEFAULT ((-9999)),
	err_r real NOT NULL DEFAULT ((-9999)),
	err_i real NOT NULL DEFAULT ((-9999)),
	err_z real NOT NULL DEFAULT ((-9999)),
	dered_u real NOT NULL DEFAULT ((-9999)),
	dered_g real NOT NULL DEFAULT ((-9999)),
	dered_r real NOT NULL DEFAULT ((-9999)),
	dered_i real NOT NULL DEFAULT ((-9999)),
	dered_z real NOT NULL DEFAULT ((-9999)),
 CONSTRAINT pk_PhotoObjAll_objID PRIMARY KEY  
(
	objID
)
);

select objID1 from "Match" where objID1 not in (select objID1 from
PhotoObjAll);

drop table "Match";
drop table PhotoObjAll;
