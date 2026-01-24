package axi4_lite_pkg;

	// =============== AXI4-Lite Master registers ==================

	typedef struct packed {
    reg        AWVALID;
    reg [31:0] AWADDR;
  } axi4_aw_Mreg;

  typedef struct packed {
    reg        WVALID;
    reg [31:0] WDATA;
    reg [3:0]  WSTRB;
  } axi4_w_Mreg;

  typedef struct packed {
    reg BREADY;
  } axi4_b_Mreg;

  typedef struct packed {
    reg        ARVALID;
    reg [31:0] ARADDR;
  } axi4_ar_Mreg;

  typedef struct packed {
    reg RREADY;
  } axi4_r_Mreg;

  typedef struct packed {
    axi4_aw_Mreg aw;
    axi4_w_Mreg  w;
    axi4_b_Mreg  b;
    axi4_ar_Mreg ar;
    axi4_r_Mreg  r;
  } axi4_lite_Mreg;

	// =============== AXI4-Lite Slave registers ==================

  typedef struct packed {
    reg AWREADY;
  } axi4_aw_Sreg;

  typedef struct packed {
    reg WREADY;
  } axi4_w_Sreg;

  typedef struct packed {
    reg        BVALID;
    reg [1:0]  BRESP;
  } axi4_b_Sreg;

  typedef struct packed {
    reg ARREADY;
  } axi4_ar_Sreg;

  typedef struct packed {
    reg        RVALID;
    reg [31:0] RDATA;
    reg [1:0]  RRESP;
  } axi4_r_Sreg;

  typedef struct packed {
    axi4_aw_Sreg aw;
    axi4_w_Sreg  w;
    axi4_b_Sreg  b;
    axi4_ar_Sreg ar;
    axi4_r_Sreg  r;
  } axi4_lite_Sreg;

endpackage

// =============== AXI4-Lite Interface ==================

interface axi4_aw_if ();
  wire AWVALID;
  wire AWREADY;
  wire [31:0] AWADDR;
//	wire [3:0] AWPROT;
endinterface

interface axi4_w_if ();
  wire WVALID;
  wire WREADY;
  wire [31:0] WDATA;
  wire [3:0] WSTRB;
endinterface

interface axi4_b_if ();
  wire BVALID;
  wire BREADY;
  wire [1:0] BRESP;
endinterface

interface axi4_ar_if ();
  wire ARVALID;
  wire ARREADY;
  wire [31:0] ARADDR;
//	wire [3:0] ARPROT;
endinterface

interface axi4_r_if ();
  wire RVALID;
  wire RREADY;
  wire [31:0] RDATA;
  wire [1:0] RRESP;
endinterface

interface axi4_lite_if ();
  wire ACLK;
  wire ARESETn;

  axi4_aw_if aw();
  axi4_w_if w();
  axi4_b_if b();
  axi4_ar_if ar();
  axi4_r_if r();

endinterface // axi4_lite_if