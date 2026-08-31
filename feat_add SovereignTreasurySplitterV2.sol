// SPDX-License-Identifier: MIT
pragma solidity ^0.8.26;

interface IERC20 {
    function transfer(address to, uint256 value) external returns (bool);
    function balanceOf(address account) external view returns (uint256);
    function transferFrom(address sender, address recipient, uint256 amount) external returns (bool);
}

/**
 * @title SovereignTreasurySplitterV2
 * @notice Multi-token programmatic allocation matrix for the UNDC sovereign grid.
 */
contract SovereignTreasurySplitterV2 {
    
    // Immutable Allocation Targets
    address public immutable regenerativeExecutionPool; 
    address public immutable sovereignLiquidityReserve; 
    address public immutable continuousDevelopmentPool; 

    // Regenerative Distribution Targets (Sub-Allocation of the 60% Pool)
    address public immutable nativeReservations;         
    address public immutable modularHousingSystems;      
    address public immutable autonomousOceanRestoration; 

    // Streaming & Token State Tracks
    uint256 public immutable streamStartTime;
    uint256 public immutable streamDuration;
    
    mapping(address => uint256) public totalStreamedTokenAllocation;
    mapping(address => uint256) public claimedStreamedTokenAllocation;

    // Basis Point Constants
    uint256 private constant REGEN_BPS = 6000;   // 60%
    uint256 private constant LIQ_BPS = 2000;     // 20%
    uint256 private constant DEV_BPS = 2000;     // 20%
    uint256 private constant TOTAL_BPS = 10000;

    // Events
    event NativeFundsSplit(address indexed sender, uint256 totalAmount, uint256 regen, uint256 liquidity, uint256 dev);
    event TokenFundsSplit(address indexed token, address indexed sender, uint256 totalAmount, uint256 regen, uint256 liquidity, uint256 dev);
    event RegenTokenDistributed(address indexed token, uint256 reservations, uint256 housing, uint256 oceans);
    event TokenStreamReleased(address indexed token, uint256 amount);

    constructor(
        address _regenPool,
        address _liqReserve,
        address _devPool,
        address _reservations,
        address _housing,
        address _oceans,
        uint256 _streamDuration
    ) {
        require(_regenPool != address(0) && _liqReserve != address(0) && _devPool != address(0), "Invalid main pool targets");
        require(_reservations != address(0) && _housing != address(0) && _oceans != address(0), "Invalid sub-pool targets");
        
        regenerativeExecutionPool = _regenPool;
        sovereignLiquidityReserve = _liqReserve;
        continuousDevelopmentPool = _devPool;
        
        nativeReservations = _reservations;
        modularHousingSystems = _housing;
        autonomousOceanRestoration = _oceans;

        streamStartTime = block.timestamp;
        streamDuration = _streamDuration;
    }

    /**
     * @notice Native fallback to automatically split incoming Ether transactions.
     */
    receive() external payable {
        uint256 amount = msg.value;
        require(amount > 0, "No value sent");

        uint256 regenShare = (amount * REGEN_BPS) / TOTAL_BPS;
        uint256 liqShare = (amount * LIQ_BPS) / TOTAL_BPS;
        uint256 devShare = amount - regenShare - liqShare; 

        payable(sovereignLiquidityReserve).transfer(liqShare);
        payable(continuousDevelopmentPool).transfer(devShare);

        totalStreamedTokenAllocation[address(0)] += regenShare;

        emit NativeFundsSplit(msg.sender, amount, regenShare, liqShare, devShare);
    }

    /**
     * @notice Processes incoming ERC-20 license fees and applies the 60/20/20 distribution split.
     * @param token Address of the ERC-20 token interface ($LLC, USDC, etc.)
     * @param amount Total token quantity to swallow and split.
     */
    function splitERC20Tokens(address token, uint256 amount) external {
        require(amount > 0, "Token quantity must exceed zero");
        require(token != address(0), "Invalid token interface");

        uint256 regenShare = (amount * REGEN_BPS) / TOTAL_BPS;
        uint256 liqShare = (amount * LIQ_BPS) / TOTAL_BPS;
        uint256 devShare = amount - regenShare - liqShare;

        require(IERC20(token).transferFrom(msg.sender, address(this), amount), "Token inbound routing failed");

        require(IERC20(token).transfer(sovereignLiquidityReserve, liqShare), "Liquidity allocation transfer failed");
        require(IERC20(token).transfer(continuousDevelopmentPool, devShare), "Development allocation transfer failed");

        totalStreamedTokenAllocation[token] += regenShare;

        emit TokenFundsSplit(token, msg.sender, amount, regenShare, liqShare, devShare);
    }

    /**
     * @notice Calculates the lineary claimable tokens/native assets locked in the reservoir.
     */
    function calculateClaimableStream(address token) public view returns (uint256) {
        if (block.timestamp <= streamStartTime) return 0;
        
        uint256 elapsedTime = block.timestamp - streamStartTime;
        if (elapsedTime >= streamDuration) {
            return totalStreamedTokenAllocation[token] - claimedStreamedTokenAllocation[token];
        }

        uint256 totalVested = (totalStreamedTokenAllocation[token] * elapsedTime) / streamDuration;
        return totalVested - claimedStreamedTokenAllocation[token];
    }

    /**
     * @notice Releases linear vested token streams to planetary restoration vectors.
     * @param token Target asset to claim and route (Address(0) for native ETH).
     */
    function releaseRegenerativeStream(address token) external {
        uint256 claimable = calculateClaimableStream(token);
        require(claimable > 0, "No vested funds available for release");

        claimedStreamedTokenAllocation[token] += claimable;

        uint256 reservationShare = claimable / 3;
        uint256 housingShare = claimable / 3;
        uint256 oceanShare = claimable - reservationShare - housingShare; 

        if (token == address(0)) {
            payable(nativeReservations).transfer(reservationShare);
            payable(modularHousingSystems).transfer(housingShare);
            payable(autonomousOceanRestoration).transfer(oceanShare);
        } else {
            require(IERC20(token).transfer(nativeReservations, reservationShare), "Reservation token routing failed");
            require(IERC20(token).transfer(modularHousingSystems, housingShare), "Housing token routing failed");
            require(IERC20(token).transfer(autonomousOceanRestoration, oceanShare), "Ocean token routing failed");
        }

        emit TokenStreamReleased(token, claimable);
        emit RegenTokenDistributed(token, reservationShare, housingShare, oceanShare);
    }

    fallback() external payable {
        revert("Direct fallback calls restricted. Use splitERC20Tokens or standard native rails.");
    }
}
