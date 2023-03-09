
`timescale 1 ns / 1 ps

    module k_aud2stream_2_v1_0 #
    (
        // Users to add parameters here
          // This is the input clock rate / 48000.
          // Default input clock = 100 MHz
        parameter integer K_CLOCK_DIVISOR       =  2083,

        parameter integer AUDIOCTRL_OUTPUT_WIDTH =  24,


        // User parameters ends
        // Do not modify the parameters beyond this line


        // Parameters of Axi Master Bus Interface M00_AXI
        parameter  C_M00_AXI_START_DATA_VALUE    = 32'hAA000000,
        parameter  C_M00_AXI_TARGET_SLAVE_BASE_ADDR    = 32'h40000000,
        parameter integer C_M00_AXI_ADDR_WIDTH    = 32,
        parameter integer C_M00_AXI_DATA_WIDTH    = 32,
        parameter integer C_M00_AXI_TRANSACTIONS_NUM    = 4,

        // Parameters of Axi Master Bus Interface M00_AXIS
        parameter integer C_M00_AXIS_TDATA_WIDTH    = 32,
        parameter integer C_M00_AXIS_START_COUNT    = 32
    )
    (
        // Users to add ports here
        output wire [C_M00_AXIS_TDATA_WIDTH-1 : 0] KDEBUG_read_data,
        output wire [C_M00_AXIS_TDATA_WIDTH-1 : 0] KDEBUG_send_data,

        // User ports ends
        // Do not modify the ports beyond this line


        // Ports of Axi Master Bus Interface M00_AXI
        input wire  m00_axi_init_axi_txn,
        output wire  m00_axi_error,
        output wire  m00_axi_txn_done,
        input wire  m00_axi_aclk,
        input wire  m00_axi_aresetn,
        output wire [C_M00_AXI_ADDR_WIDTH-1 : 0] m00_axi_awaddr,
        output wire [2 : 0] m00_axi_awprot,
        output wire  m00_axi_awvalid,
        input wire  m00_axi_awready,
        output wire [C_M00_AXI_DATA_WIDTH-1 : 0] m00_axi_wdata,
        output wire [C_M00_AXI_DATA_WIDTH/8-1 : 0] m00_axi_wstrb,
        output wire  m00_axi_wvalid,
        input wire  m00_axi_wready,
        input wire [1 : 0] m00_axi_bresp,
        input wire  m00_axi_bvalid,
        output wire  m00_axi_bready,
        output wire [C_M00_AXI_ADDR_WIDTH-1 : 0] m00_axi_araddr,
        output wire [2 : 0] m00_axi_arprot,
        output wire  m00_axi_arvalid,
        input wire  m00_axi_arready,
        input wire [C_M00_AXI_DATA_WIDTH-1 : 0] m00_axi_rdata,
        input wire [1 : 0] m00_axi_rresp,
        input wire  m00_axi_rvalid,
        output wire  m00_axi_rready,

        // Ports of Axi Master Bus Interface M00_AXIS
        input wire  m00_axis_aclk,
        input wire  m00_axis_aresetn,
        output wire  m00_axis_tvalid,
        output wire [C_M00_AXIS_TDATA_WIDTH-1 : 0] m00_axis_tdata,
        output wire [(C_M00_AXIS_TDATA_WIDTH/8)-1 : 0] m00_axis_tstrb,
        output wire  m00_axis_tlast,
        input wire  m00_axis_tready
    );
    localparam integer FFT_RE_IM_WIDTH       =  C_M00_AXI_DATA_WIDTH/2;

// Instantiation of Axi Bus Interface M00_AXI
    k_aud2stream_2_v1_0_M00_AXI # (
        .K_AUDIO_REGISTER_TO_READ(0),
        .C_M_START_DATA_VALUE(C_M00_AXI_START_DATA_VALUE),
        .C_M_TARGET_SLAVE_BASE_ADDR(C_M00_AXI_TARGET_SLAVE_BASE_ADDR),
        .C_M_AXI_ADDR_WIDTH(C_M00_AXI_ADDR_WIDTH),
        .C_M_AXI_DATA_WIDTH(C_M00_AXI_DATA_WIDTH),
        .C_M_TRANSACTIONS_NUM(C_M00_AXI_TRANSACTIONS_NUM)
    ) k_aud2stream_2_v1_0_M00_AXI_inst (
        //.INIT_AXI_TXN(m00_axi_init_axi_txn),
        .INIT_AXI_TXN(init_txn),
        .READ_DATA(k_read_data),
        .ERROR(m00_axi_error),
        .TXN_DONE(m00_axi_txn_done),
        .M_AXI_ACLK(m00_axi_aclk),
        .M_AXI_ARESETN(m00_axi_aresetn),
        .M_AXI_AWADDR(m00_axi_awaddr),
        .M_AXI_AWPROT(m00_axi_awprot),
        .M_AXI_AWVALID(m00_axi_awvalid),
        .M_AXI_AWREADY(m00_axi_awready),
        .M_AXI_WDATA(m00_axi_wdata),
        .M_AXI_WSTRB(m00_axi_wstrb),
        .M_AXI_WVALID(m00_axi_wvalid),
        .M_AXI_WREADY(m00_axi_wready),
        .M_AXI_BRESP(m00_axi_bresp),
        .M_AXI_BVALID(m00_axi_bvalid),
        .M_AXI_BREADY(m00_axi_bready),
        .M_AXI_ARADDR(m00_axi_araddr),
        .M_AXI_ARPROT(m00_axi_arprot),
        .M_AXI_ARVALID(m00_axi_arvalid),
        .M_AXI_ARREADY(m00_axi_arready),
        .M_AXI_RDATA(m00_axi_rdata),
        .M_AXI_RRESP(m00_axi_rresp),
        .M_AXI_RVALID(m00_axi_rvalid),
        .M_AXI_RREADY(m00_axi_rready)
    );

// Instantiation of Axi Bus Interface M00_AXIS
    k_aud2stream_2_v1_0_M00_AXIS # (
        .C_M_AXIS_TDATA_WIDTH(C_M00_AXIS_TDATA_WIDTH),
        .C_M_START_COUNT(C_M00_AXIS_START_COUNT)
    ) k_aud2stream_2_v1_0_M00_AXIS_inst (
        .DATA_TO_SEND(k_send_data),
        .INIT_AXIS_TXN(init_txn),
        .M_AXIS_ACLK(m00_axis_aclk),
        .M_AXIS_ARESETN(m00_axis_aresetn),
        .M_AXIS_TVALID(m00_axis_tvalid),
        .M_AXIS_TDATA(m00_axis_tdata),
        .M_AXIS_TSTRB(m00_axis_tstrb),
        .M_AXIS_TLAST(m00_axis_tlast),
        .M_AXIS_TREADY(m00_axis_tready)
    );

    ////////////////////////////////////////////////
    ////////////////////////////////////////////////
    ////////////////////////////////////////////////

    // Add user logic here
    wire  init_txn;
    wire [C_M00_AXIS_TDATA_WIDTH-1 : 0] k_read_data;
    wire [C_M00_AXIS_TDATA_WIDTH-1 : 0] k_send_data;

    // DEBUGGING
    assign KDEBUG_send_data = k_send_data;
    assign KDEBUG_read_data = k_read_data;

    // Copy the upper bits of the data into the REAL fields.
    // See this for more info: https://docs.xilinx.com/r/en-US/pg109-xfft/TDATA-Example?tocId=0Bg0hBCYZAqnRwPR7IMVBg
    // NOTE: KNOWN ISSUE - we don't pad properly if the FFT_RE_IM_WIDTH is not a multiple of 8.
    assign k_send_data[FFT_RE_IM_WIDTH-1 : 0]                                                  = k_read_data[AUDIOCTRL_OUTPUT_WIDTH - 1 : AUDIOCTRL_OUTPUT_WIDTH - FFT_RE_IM_WIDTH];
    assign k_send_data[C_M00_AXIS_TDATA_WIDTH - 1 : C_M00_AXIS_TDATA_WIDTH - FFT_RE_IM_WIDTH]  = {FFT_RE_IM_WIDTH{1'b0}};

    // 24 bits is big enough.
    reg [24 : 0] clk_divider_ctr;
    reg audio_clk;

    // Clock divider:
    always @(posedge m00_axi_aclk)
    begin
        clk_divider_ctr <= clk_divider_ctr + 24'd1;
        if (clk_divider_ctr >= (K_CLOCK_DIVISOR-1))
           clk_divider_ctr <= 24'd0;
        audio_clk <= (clk_divider_ctr < K_CLOCK_DIVISOR/2) ? 1'b1 : 1'b0;
    end

    // Tell the AXI master to read every time the audio clock ticks
    assign init_txn = audio_clk;


    // User logic ends

    endmodule
