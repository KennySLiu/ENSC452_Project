----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 02/06/2023 05:05:41 PM
-- Design Name: 
-- Module Name: fft_testbench - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


--- NOTE(Kenny): To make this, I referenced this video: https://www.youtube.com/watch?v=kfaHtXxtQmI

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.Numeric_STD.all;


-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity fft_testbench is
end fft_testbench;


architecture Behavioral of fft_testbench is

    component xil_fft is
      port (
        M_AXIS_DATA_0_tdata : out STD_LOGIC_VECTOR ( 95 downto 0 );
        M_AXIS_DATA_0_tlast : out STD_LOGIC;
        M_AXIS_DATA_0_tready : in STD_LOGIC;
        M_AXIS_DATA_0_tvalid : out STD_LOGIC;
        S_AXIS_CONFIG_0_tdata : in STD_LOGIC_VECTOR ( 23 downto 0 );
        S_AXIS_CONFIG_0_tready : out STD_LOGIC;
        S_AXIS_CONFIG_0_tvalid : in STD_LOGIC;
        S_AXIS_DATA_0_tdata : in STD_LOGIC_VECTOR ( 95 downto 0 );
        S_AXIS_DATA_0_tlast : in STD_LOGIC;
        S_AXIS_DATA_0_tready : out STD_LOGIC;
        S_AXIS_DATA_0_tvalid : in STD_LOGIC;
        aclk_0 : in STD_LOGIC;
        aclken_0 : in STD_LOGIC;
        aresetn_0 : in STD_LOGIC;
        event_data_in_channel_halt_0 : out STD_LOGIC;
        event_data_out_channel_halt_0 : out STD_LOGIC;
        event_frame_started_0 : out STD_LOGIC;
        event_status_channel_halt_0 : out STD_LOGIC;
        event_tlast_missing_0 : out STD_LOGIC;
        event_tlast_unexpected_0 : out STD_LOGIC
      );
    end component;
    
    -- Procedure for clock generation (SOURCE: https://stackoverflow.com/questions/17904514/vhdl-how-should-i-create-a-clock-in-a-testbench)
    procedure clk_gen(signal clk : out std_logic; constant FREQ : real) is
      constant PERIOD    : time := 1 sec / FREQ;        -- Full period
      constant HIGH_TIME : time := PERIOD / 2;          -- High time
      constant LOW_TIME  : time := PERIOD - HIGH_TIME;  -- Low time; always >= HIGH_TIME
    begin
      -- Check the arguments
      assert (HIGH_TIME /= 0 fs) report "clk_plain: High time is zero; time resolution to large for frequency" severity FAILURE;
      -- Generate a clock cycle
      loop
        clk <= '1';
        wait for HIGH_TIME;
        clk <= '0';
        wait for LOW_TIME;
      end loop;
    end procedure;

    -- Clock frequency and signal
    signal clk_48k : std_logic;
    signal clk_200m : std_logic;

    ----------------------------------------
    -- INPUT DATA:
    -- 2 channels, 24-bits each, because we configured our block with 24-bit input, and 2 channels.
    signal IN_DATA_CH1_RE   : STD_LOGIC_VECTOR(23 downto 0) := (others => '0');
    signal IN_DATA_CH1_IM   : STD_LOGIC_VECTOR(23 downto 0) := (others => '0');
    signal IN_DATA_CH2_RE   : STD_LOGIC_VECTOR(23 downto 0) := (others => '0');
    signal IN_DATA_CH2_IM   : STD_LOGIC_VECTOR(23 downto 0) := (others => '0');

    ----------------------------------------
    -- SIGNALS to connect directly to the FFT block:
    ----------------------------------------
    --signal aclk_0                              : in STD_LOGIC := '0';
    signal M_AXIS_DATA_0_tdata                 : STD_LOGIC_VECTOR ( 95 downto 0 )   := (others => '0');
    signal S_AXIS_CONFIG_0_tdata               : STD_LOGIC_VECTOR ( 23 downto 0 )   := (others => '0');
    signal S_AXIS_DATA_0_tdata                 : STD_LOGIC_VECTOR ( 95 downto 0 )   := (others => '0');
    signal M_AXIS_DATA_0_tlast                 : STD_LOGIC                          := '0';
    signal M_AXIS_DATA_0_tready                : STD_LOGIC                          := '0';
    signal M_AXIS_DATA_0_tvalid                : STD_LOGIC                          := '0';
    signal S_AXIS_CONFIG_0_tready              : STD_LOGIC                          := '0';
    signal S_AXIS_CONFIG_0_tvalid              : STD_LOGIC                          := '0';
    signal S_AXIS_DATA_0_tlast                 : STD_LOGIC                          := '0';
    signal S_AXIS_DATA_0_tready                : STD_LOGIC                          := '0';
    signal S_AXIS_DATA_0_tvalid                : STD_LOGIC                          := '0';
    signal aclken_0                            : STD_LOGIC                          := '0';
    signal aresetn_0                           : STD_LOGIC                          := '0';
    signal event_data_in_channel_halt_0        : STD_LOGIC                          := '0';
    signal event_data_out_channel_halt_0       : STD_LOGIC                          := '0';
    signal event_frame_started_0               : STD_LOGIC                          := '0';
    signal event_status_channel_halt_0         : STD_LOGIC                          := '0';
    signal event_tlast_missing_0               : STD_LOGIC                          := '0';
    signal event_tlast_unexpected_0            : STD_LOGIC                          := '0';


begin

    clk_gen(clk_48k, 48000.0);
    clk_gen(clk_200m, 200.00000e6);

    FFT_BLK : xil_fft port map(
        M_AXIS_DATA_0_tdata             => M_AXIS_DATA_0_tdata             ,
        M_AXIS_DATA_0_tlast             => M_AXIS_DATA_0_tlast             ,
        M_AXIS_DATA_0_tready            => M_AXIS_DATA_0_tready            ,
        M_AXIS_DATA_0_tvalid            => M_AXIS_DATA_0_tvalid            ,
        S_AXIS_CONFIG_0_tdata           => S_AXIS_CONFIG_0_tdata           ,
        S_AXIS_CONFIG_0_tready          => S_AXIS_CONFIG_0_tready          ,
        S_AXIS_CONFIG_0_tvalid          => S_AXIS_CONFIG_0_tvalid          ,
        S_AXIS_DATA_0_tdata             => S_AXIS_DATA_0_tdata             ,
        S_AXIS_DATA_0_tlast             => S_AXIS_DATA_0_tlast             ,
        S_AXIS_DATA_0_tready            => S_AXIS_DATA_0_tready            ,
        S_AXIS_DATA_0_tvalid            => S_AXIS_DATA_0_tvalid            ,
        aclk_0                          => clk_200m                        ,    -- Clock comes from our 200Mhz clk
        aclken_0                        => aclken_0                        ,
        aresetn_0                       => aresetn_0                       ,
        event_data_in_channel_halt_0    => event_data_in_channel_halt_0    ,
        event_data_out_channel_halt_0   => event_data_out_channel_halt_0   ,
        event_frame_started_0           => event_frame_started_0           ,
        event_status_channel_halt_0     => event_status_channel_halt_0     ,
        event_tlast_missing_0           => event_tlast_missing_0           ,
        event_tlast_unexpected_0        => event_tlast_unexpected_0        
    );

    init_blk: process(clk_200m)
        variable clk_ctr : integer range 0 to 100000;
    begin
        if rising_edge(clk_200m) then
            clk_ctr := clk_ctr + 1;

            if (clk_ctr > 10) then
                aclken_0 <= '1';
                aresetn_0 <= '1';
            end if;
        end if;

    end process;
    
    
    DC_Stimulus: process(clk_200m, clk_48k)
        --variable clk_48k_just_rose : integer range 0 to 1;
        variable clk_ctr : integer range 0 to 100000;
    begin
        -- Don't bother simulating complex inputs - only real inputs allowed!
        IN_DATA_CH1_IM <= (others => '0');
        IN_DATA_CH2_IM <= (others => '0');
        S_AXIS_DATA_0_tready <= '1';

        if rising_edge(clk_200m) then
            clk_ctr := clk_ctr + 1;
            
            if (clk_ctr > 500) then
                IN_DATA_CH1_RE <= std_logic_vector(to_signed(5000, IN_DATA_CH1_RE'length));
                IN_DATA_CH2_RE <= std_logic_vector(to_signed(5000, IN_DATA_CH2_RE'length));
    
                S_AXIS_DATA_0_tdata <= IN_DATA_CH2_IM &
                                       IN_DATA_CH2_RE &
                                       IN_DATA_CH1_IM & 
                                       IN_DATA_CH1_RE;
    
                S_AXIS_DATA_0_tvalid <= '1';
            end if;
 
        end if;
    end process;




end Behavioral;































